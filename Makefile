main: main.cpp TreeNode.cpp ZMQserver.h zmqhelp.h Request.h ping.h customexceptions.h MySemaphore.h Response.h
	g++ -Wall -o treenode TreeNode.cpp -lzmq
	g++ -Wall -o main main.cpp -lzmq -pthread

clean:
	rm -rf ./main ./treenode