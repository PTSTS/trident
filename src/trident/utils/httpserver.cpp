#include <trident/utils/httpserver.h>

#include <kognac/logs.h>

#include <chrono>

#if defined(_WIN32)
//The Http Client and Server are only supported under Linux/Mac
#else

namespace chr = std::chrono;

HttpServer::HttpServer(uint32_t port,
        std::function<void(const std::string&, std::string&)> handler,
        uint32_t nthreads,
        uint64_t maxLifeConn) : port(port),
    maxLifeConn(maxLifeConn), handlerFunction(handler) {
        threads.resize(nthreads);
        for(uint32_t i = 0; i < nthreads; ++i) {
            threads[i] = std::thread(&HttpServer::processSocket, this);
        }
    }

uint64_t HttpServer::getMessageBodyLength(std::string& request) {
    size_t pos = request.find("Content-Length:");
    if (pos == std::string::npos) {
        // For GET requests, usually message body is not present. Hence Content-Length field can be omitted
        // For POST requests, if message body is not present, then it Content-Length is 0
        return 0;
    }
    size_t endpos = request.find("\r\n", pos);
    std::string bodyLen = request.substr(pos + 16, endpos - pos - 16);
    uint64_t ret = std::stoul(bodyLen);
    return ret;
}

void HttpServer::processSocket() {
    while (true) {
        Socket socket;
        queueReady.pop_wait(socket);
        if (socket.isEOF()) {
            break;
        }

        int connFd = socket.getFD();
        //Read the data and print it on screen
        try {
            char buffer[1024];
            std::string request = "";
            auto len = read(connFd, buffer, 1024);
            if (len == 0) { //Client has closed the conversation
                socket.close();
                queueWait.push(socket);
            } else if (len == -1) {
                //can happen. Put it back in the queue, unless there are errors
                //TODO
                queueWait.push(socket);
            } else {
                request += std::string(buffer, len);
                int bodyLength = getMessageBodyLength(request);
                int pos = request.find("\r\n\r\n");
                int headerLength = 0;
                if (pos == std::string::npos) {
                    LOG(ERRORL) << "FATAL: End of header not found in the request";
                }else {
                    headerLength = pos;
                }
                if (bodyLength > 0) {
                    int readBuffer = bodyLength - (1024 - headerLength);
                    while (readBuffer > 0) {
                        auto len = read(connFd, buffer, 1024);
                        readBuffer -= len;
                        request += std::string(buffer, len);
                    }
                }
                std::string response = "";
                handlerFunction(request, response);
                uint64_t size = 0;
                do {
                    size += send(connFd, response.c_str() + size,
                            response.size() - size, 0);
                } while (size < response.size());
                queueWait.push(socket);
            }
        } catch (...) {
            if (connFd != -1) {
                socket.close();
                queueWait.push(socket);
            }
        }
    }
}

void HttpServer::waitForData() {
    std::vector<pollfd> events;
    std::vector<uint64_t > idletime;

    while (true) {
        while (!queueWait.isEmpty()) {
            //Process the sockets
            Socket s;
            queueWait.pop(s);
            if (s.shouldBeClosed()) {
                //Remove the FD from the list of monitored ones
                for(auto &el : events) {
                    if (el.fd == s.getFD()) {
                        el.fd = -1;
                        break;
                    }
                }
                //Close the FD
                shutdown(s.getFD(), 2);
                close(s.getFD());
            } else {
                uint64_t time = chr::duration_cast<chr::milliseconds>(
                        chr::steady_clock::now().time_since_epoch()).count();
                //Is the FD already monitored?
                bool found = false;
                int validIdx = -1;
                int i = 0;
                for(auto &el : events) {
                    if (el.fd == s.getFD()) {
                        found = true;
                        //Update its idle count
                        idletime[i] = time;
                        break;
                    } else if (el.fd == -1) {
                        validIdx = i;
                    }
                    i++;
                }
                if (!found) {

                    //Reuse a valid slot or add a new one
                    if (validIdx == -1) {
                        pollfd pf;
                        pf.fd = s.getFD();
                        pf.events = POLLIN | POLLERR | POLLHUP;
                        events.push_back(pf);
                        idletime.push_back(time);
                    } else {
                        events[validIdx].fd = s.getFD();
                        idletime[validIdx] = time;
                    }
                }
            }
        }

        int retval = poll(events.data(), events.size(), 1000); //Wait max 1sec
        if (retval > 0) {
            //Add to queueReady all sockets which have data
            for(int i = 0; i < events.size(); ++i) {
                auto &el = events[i];
                if (el.fd == -1)
                    continue;
                if (el.revents & POLLIN) {
                    queueReady.push(Socket(el.fd));
                    el.fd = -1; //Remove it from the list...
                } else if (el.revents != 0) {
                    shutdown(el.fd, 2);
                    close(el.fd);
                    el.fd = -1;
                } else {
                    //No update ...
                }
            }
        } else if (retval == 0) {
            if (!events.empty() && events.back().fd == -1) {
                events.pop_back();
                idletime.pop_back();
            }
            //Remove connections that were idle for too long
            uint64_t time = chr::duration_cast<chr::milliseconds>(
                    chr::steady_clock::now().time_since_epoch()).count();
            for(int i = 0; i < events.size(); ++i) {
                if (events[i].fd != -1) {
                    if ((idletime[i] + maxLifeConn) < time) {
                        //Remove it...
                        shutdown(events[i].fd, 2);
                        close(events[i].fd);
                        events[i].fd = -1;
                    }
                }
            }
        } else {
            LOG(ERRORL) << "Pool failed";
        }
    }
}

bool HttpServer::listn() {
    //Create the socket
    listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listenFd < 0) {
        return false;
    }

    bzero((char*) &svrAdd, sizeof(svrAdd));
    svrAdd.sin_family = AF_INET;
    svrAdd.sin_addr.s_addr = INADDR_ANY;
    svrAdd.sin_port = htons(port);
    if(bind(listenFd, (struct sockaddr *)&svrAdd, sizeof(svrAdd)) < 0) {
        return false;
    }
    listen(listenFd, 10);

    //Start the thread that process the incoming sockets
    threadWait = std::thread(&HttpServer::waitForData, this);
    launched = true;
    while (true) {
        //this is where client connects.
        socklen_t len = sizeof(clntAdd);
        int connFd = accept(listenFd, (struct sockaddr *)&clntAdd, &len);
        if (connFd < 0) {
            return false;
        }
        //Make the socket non-blocking
        //fcntl(connFd, F_SETFL, fcntl(connFd, F_GETFL, 0) | O_NONBLOCK);
        queueReady.push(Socket(connFd));
    }
    launched = false;
}

void HttpServer::start() {
    launched = false;
    listn();
}

void HttpServer::stop() {
    //Stop all processing threads
    for(int i = 0; i < threads.size(); ++i)
        queueReady.push(Socket());
    for(auto &t : threads) {
        t.join();
    }
    close(listenFd);
    while (launched) {
        //TODO Wait until the thread has stopped
    }
}

#endif
