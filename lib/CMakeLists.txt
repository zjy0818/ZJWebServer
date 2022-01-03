set(ZJWebServer_lib
            acceptor.c
            buffer.c
            channel.c
            channel_map.c
            event_loop.c
            event_loop_thread.c
            epoll_dispatcher.c
            http_request.c
            http_response.c
            http_server.c
            tcp_server.c
            tcp_connection.c
            thread_pool.c
            utils.c )

add_library(ZJWebServer ${ZJWebServer_lib})
target_link_libraries(ZJWebServer pthread)

install(TARGETS ZJWebServer DESTINATION lib)

#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/)

