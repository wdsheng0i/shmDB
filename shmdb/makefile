Shm : InputHandle.o TMutex.o tinyxmlparser.o tinyxmlerror.o tinyxml.o tinystr.o TableHandle.o ShmHandle.o PageHandle.o DiskFileHandle.o BitMap.o
	g++ -o Shm InputHandle.o TMutex.o tinyxmlparser.o tinyxmlerror.o tinyxml.o tinystr.o TableHandle.o ShmHandle.o PageHandle.o DiskFileHandle.o BitMap.o


BitMap.o : BitMap.cpp BitMap.h
	g++ -c BitMap.cpp
ShmHandle.o : ShmHandle.cpp ShmHandle.h DiskFileHandle.h  PageHandle.h ShmStruct.h
	g++ -c ShmHandle.cpp
PageHandle.o : PageHandle.cpp PageHandle.h TableHandle.h  BitMap.h ShmStruct.h
	g++ -c PageHandle.cpp
DiskFileHandle.o : DiskFileHandle.cpp DiskFileHandle.h ShmStruct.h
	g++ -c DiskFileHandle.cpp
TableHandle.o : TableHandle.cpp TableHandle.h PageHandle.h  BitMap.h ShmStruct.h
	g++ -c TableHandle.cpp
tinystr.o : tinystr.cpp tinystr.h 
	g++ -c tinystr.cpp
tinyxml.o : tinyxml.cpp tinyxml.h 
	g++ -c tinyxml.cpp
tinyxmlerror.o : tinyxmlerror.cpp tinyxml.h 
	g++ -c tinyxmlerror.cpp
tinyxmlparser.o : tinyxmlparser.cpp tinyxml.h 
	g++ -c tinyxmlparser.cpp
TMutex.o : TMutex.cpp TMutex.h 
	g++ -c TMutex.cpp
InputHandle.o : InputHandle.cpp ShmHandle.h DiskFileHandle.h PageHandle.h TableHandle.h xml.h
	g++ -c InputHandle.cpp

clean :
	rm  InputHandle.o TMutex.o tinyxmlparser.o tinyxmlerror.o tinyxml.o tinystr.o TableHandle.o ShmHandle.o PageHandle.o DiskFileHandle.o BitMap.o

