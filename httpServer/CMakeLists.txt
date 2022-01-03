add_executable(HttpServer HttpServer.c)
target_link_libraries(HttpServer ZJWebServer)

add_executable(HttpServer_test HttpServer_test.c)
target_link_libraries(HttpServer_test ZJWebServer)
