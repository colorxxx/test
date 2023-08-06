#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <arpa/inet.h> 

using namespace std;

#define PORT 8080
#define FIRST_LINE_LENGTH 20
#define MAX_MESSAGE_LENGTH 50000
#define FILE_UPLOADED "/root/test/Week8/Week8/file_uploaded/"

void do_something(string buffer, string boundary);
string find_boundary(string full_boundary);

string http_header_OK = "HTTP/1.1 200 Ok\r\n";
string http_header_context = "Content-Type: application/json\r\n\r\n";
string http_header_content_length = "Date: Fri, 15 Jul 2022 11:03:12 GMT\r\n\r\n";
string new_lines = "\r\n";

int main(int argc, char const *argv[])
{
	ifstream input_file;
	string j2 = "{\"name\": \"yunjin\"}";
	int server_fd = 0;
    int new_socket = 0;
	long valread = 0;
	struct sockaddr_in address;
    struct sockaddr_in* client_Addr = NULL;
    struct in_addr ipAddr;
    char* request = NULL;
    char str[INET_ADDRSTRLEN] = {0,};
	char c_buffer[MAX_MESSAGE_LENGTH] = {0,};
    string str_address = "";
    string s_buffer = "";
	string s_filecontext = "";
	string s_filename = "";
	string s_header = "";
	string s_boundary = "";
    string parse_string = "";
	string line = "";
	stringstream ss_fullMessage("");
	stringstream ss_filecontext("");
	stringstream ss_filename("");
	stringstream ss_header("");
	int addrlen = sizeof(address);
	struct timeval tv;
	fd_set fds, copyfds;
	int result = 0;

	FD_ZERO(&fds);

	////////////////////////////////////
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("In sockets");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	memset(address.sin_zero, '\0', sizeof address.sin_zero);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 10) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}

    while(1) {
		ss_header.str("");
		ss_filename.str("");
		ss_filecontext.str("");
		ss_fullMessage.str("");
		s_header.clear();
		cout << endl << "+++++++ Waiting for new connection ++++++++" << endl << endl;
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		client_Addr = (struct sockaddr_in*)&address;
		ipAddr = client_Addr->sin_addr;
		inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
		str_address = str;
		cout << "CLIENT IP : " << str_address << endl;
		
		valread = read(new_socket, c_buffer, MAX_MESSAGE_LENGTH);
		if(valread == 0) {
			cerr << endl << "CLIENT TERMINATED ... " << endl;
			return -1;
		}
		s_buffer = c_buffer;
		cout << "-------------------- Client Message -------------------" << endl << s_buffer << endl;
		request = strtok(c_buffer, " ");
		
		if(strncmp(request, "GET", 3) == 0) {
			cout << "Client Request for : " << request << endl;
			parse_string = strtok(NULL, " "); // "GET /test.txt" 에서 /test.txt 추출
			parse_string = parse_string.substr(1, parse_string.length()); // "/test.txt"에서 "test.txt" 추출
			cout << "Client ask for path: " << parse_string << endl;

			ss_filename << FILE_UPLOADED << parse_string;
			cout << "OPEN FILE " << ss_filename.str() << endl;
			input_file.open(ss_filename.str().c_str());
			if(!input_file.is_open()) {
				cerr << endl << "ERROR : COULD NOT OPEN FILE " << parse_string << endl;
				return -1;
			}
			while(getline(input_file, line)) {
				cout << " >>>>>>>>>> " << line << "<<<<<<<<<<" << endl; 
				ss_filecontext << line;
			}
			input_file.close();
			// while(!input_file.eof()) {
			// 	line.clear();
			// 	input_file >> line;
			// 	ss_filecontext << line;
			// }
			ss_fullMessage << http_header_OK << http_header_content_length;
			
			write(new_socket, ss_fullMessage.str().c_str(), ss_fullMessage.str().length());
			write(new_socket, ss_filecontext.str().c_str(), ss_filecontext.str().length());

			cout << "----------server header----------" << endl << http_header_OK;
			cout << "----------server message---------" << endl << ss_filecontext.str() << endl;
			cout << "---------------------------------" << endl;
			
		} else if(strncmp(request, "POST", 4) == 0) {
			// POST 일 때
			// boundary 추출
			s_boundary = find_boundary(s_buffer);

			// select를 통해 chunk 입력받음 
			FD_SET(new_socket, &fds);
			tv.tv_sec = 2;

			while(1) {
				cout << "+++++++++++++++++++ NEW CHUNK +++++++++++++++++++" << endl;
				ss_filecontext.str("");
				copyfds = fds;
				result = select(new_socket + 1, &copyfds, NULL, NULL, &tv);
				if(result == 0) {
					break;
				}
				valread = read(new_socket, c_buffer, MAX_MESSAGE_LENGTH);
				if(valread <= 0) {
					break;
				}
				cout << c_buffer << endl;
				s_buffer = c_buffer;
				// boundary 추출 및 파일명 , 파일 형식, 파일 내용 저장
				do_something(s_buffer, s_boundary);
			}
			cout << "-------------------- Client Message END -------------------" << endl;
			
			ss_header << http_header_OK << http_header_context;

			write(new_socket, ss_header.str().c_str(), ss_header.str().length());
			cout << "----------server header----------" << endl << http_header_OK;
			cout << "---------------------------------" << endl;

		}
		close(new_socket);
	}
	return 0;
}

void do_something(string buffer, string boundary) 
{
	ofstream newfile;
	string line = "";
	stringstream ss_buffer("");
	stringstream ss_endBoundary("");
	stringstream ss_filename("");
	size_t nPos = 0;
	string filename = "";
	string filecontent = "";
	string s_contentType = "Content-Type:";
	string s_fileName = "filename=";
	string s_parsedBoundary = "";

	ss_buffer << buffer;
	s_parsedBoundary = boundary.substr(0, boundary.length() - 3);

	// 첫 번째 줄 처리 (boundary 인지 확인)
	getline(ss_buffer, line);
	
	while(getline(ss_buffer, line)) {
		// filename 확인 -> data 확인 -> 파일 생성 및 데이터 입력 -> boundary인지 확인 
		nPos = line.find("filename=");
		if(nPos == string::npos) {
			break;
		}
		filename = line.substr(nPos + 9); // "file.txt" (큰따옴표까지 포함됨)
		filename = filename.substr(1, filename.length() - 3);
		ss_filename << FILE_UPLOADED << filename;
		// 파일명으로 파일을 생성하고
		newfile.open(ss_filename.str().c_str());
		// 데이터를 찾는다. (다음 줄을 읽는다)
		getline(ss_buffer, line);
		nPos = line.find(s_contentType.c_str());
		if(nPos != string::npos) {
			// content-type 줄이 있다면 한줄 띄우고부터 데이터
			getline(ss_buffer, line);
		}
		while(getline(ss_buffer, line)) {
			nPos = line.rfind(boundary.c_str());
			if(nPos != string::npos) {
				break;
			} else {
				nPos = line.rfind(s_parsedBoundary.c_str());
				if(nPos != string::npos) {
					break;
				} 
			}
			newfile << line;
		}
		newfile.close();
		ss_filename.str("");
	}
}

string find_boundary(string full_boundary)
{
	string boundary = "";
	string line = "";
	size_t locate_boundary = 0 ;
	size_t nPos = 0;
	stringstream if_boundary;
	
	if_boundary << full_boundary;

	while(getline(if_boundary, line)) {
		nPos = line.find("boundary=");
		if(nPos != string::npos) {
			locate_boundary = line.find_last_of('-');
			boundary = line.substr(locate_boundary + 1);
			break;
		}
	}
	return boundary;
}
