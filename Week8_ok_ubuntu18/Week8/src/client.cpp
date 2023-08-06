#include <iostream>
#include <string>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <curl/curl.h>
// #include "util.h"

using namespace std;

bool getHTTP(string IdPwd, string urlGet);
bool postHTTP(string urlPost);
size_t CallBackFunction(void* ptr, size_t size, size_t nmemb, void* userp);

struct MemoryStruct {
    char *memory;
    size_t size;
};

int main()
{
    string url_get = "http://localhost:8080/";
    string ini_path = "/root/test/Week/Week8/include/config.ini";
    string option = "";

    stringstream ssidpwd;
    
    cout << "====================================================================================" << endl;
    cout << "Connect to " << url_get << " ..." << endl << endl;
    // ssidpwd << parseIni("ID", ini_path) << ":" << parseIni("PWD", ini_path);
    // cout << ssidpwd.str() << endl;

    while(1) {
        cout << endl << "> Choose GET or POST : ";
        cin >> option;
    
        if(option == "GET" || option == "get") {
            getHTTP(ssidpwd.str(), url_get);
        } else if (option == "POST" || option == "post") {
            postHTTP(url_get);
        } else if (option == "Q" || option == "q") {
            cout << endl << "EXIT PROGRAM ... " << endl;
            break;
        } else {
            cout << endl << "Please check and enter again ... " << endl;
        }
    }
    return 0;
}

bool getHTTP(string IdPwd, string urlGet)
{
    CURL *curl = NULL;
    FILE *fd_data = NULL;
    FILE *fd_header = NULL;
    long statLong = 0;
    string filename = "";
    stringstream ss_urlGet("");
    CURLcode res;
    struct curl_slist *responseHeaders = NULL;
    // struct MemoryStruct chunk;
    
    // 초기화
    // chunk.memory = (char *)malloc(1);
    // chunk.size = 0;

    fd_data = fopen("/root/test/Week8/Week8/bin/get_data.txt", "w+");
    fd_header = fopen("/root/test/Week8/Week8/bin/get_header.txt", "w+");

    // context 객체 생성
    curl = curl_easy_init();
    if(!curl) {
        cerr << "curl_easy_init() failed" << endl;
        curl_global_cleanup();
        return false;
    }
    curl_global_init(CURL_GLOBAL_ALL);

    cout << "> Enter file name to GET : " ;
    cin >> filename;
    ss_urlGet << urlGet << filename;

    // METHOD : GET
    if(curl) {
        // context 객체 설정 (CURLOPT_URL은 목표 URL)
        // 마지막 인자에 string은 에러남 -> char array로 변환
        curl_easy_setopt(curl, CURLOPT_URL, ss_urlGet.str().c_str());
        // redicrection 처리 (HTTP_CODE 302)
        // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        // id/pwd 입력
        // curl_easy_setopt(curl, CURLOPT_USERPWD, IdPwd.c_str());
        
        // writefunction에 function 등록하면 callback으로 메시지를 받게된다.
        // callback 함수가 없으면 stdout 으로 출력된다. 
        // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallBackFunction);
        // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

        // chunk(MemoryStuct 구조체)를 콜백 function에게 넘겨줌
        // curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
        // curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd_data);

        // header는 표준에러로 출력
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, fd_header);

        // responseHeaders = curl_slist_append(responseHeaders, "Accept: image/png");
        // responseHeaders = curl_slist_append(responseHeaders, "Content-Type: image/png");
        // responseHeaders = curl_slist_append(responseHeaders, "Accept: application/json");
        // responseHeaders = curl_slist_append(responseHeaders, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, responseHeaders);
        
        //EXECUTE (실제 페이지 긁어오는 행위)
        cout << "------- GET " << filename << " -----------" << endl;
        res = curl_easy_perform(curl);
        cout << endl << "---------------------------------" << endl;

        if(res != CURLE_OK) {
            cerr << "CURL_EASY_PERFORM() FAILED : " << curl_easy_strerror(res) << endl;
            return false;
        } else {
            // HTTP 응답 코드(200, 404 등)를 가져오려면 CURLINFO_HTTP_CODE
            res = curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &statLong);
            if(statLong == 401) {
                // unauthorized
                cerr << "[ERROR] 401 Unauthorized : Check ID, PWD ... " << endl;
                return false;
            } else if(statLong == 404) {
                // Not Found
                cerr << "[ERROR] 404 NOT FOUND ... " << endl;
                return false;
            } else {
                cout << endl << ">> curl_easy_perform ended with " << res << endl;
                cout << ">> HTTP GET response code : " << statLong << endl;

                //받아온 데이터 처리
                // 전송 받은 문서의 크기 가져오려면 CURLINFO_SIZE_DOWNLOAD
                curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &statLong);
                // cout << "HTTP document size : " << statLong << endl;
                // cout << "읽어온 메모리 크기 : " << strlen(chunk.memory) << endl << endl;

                // header, body 출력
                // cout << "===========HEADER===========" << endl << header << "==========================" << endl;
            }
            
        }
    }
    // curl_easy_setopt 객체 소멸
    curl_easy_cleanup(curl);
    curl_slist_free_all(responseHeaders);
    curl_global_cleanup();
    fclose(fd_data);
    fclose(fd_header);
    return true;
}

bool postHTTP(string urlPost)
{
    CURL *curl = NULL;
    struct curl_slist *responseHeaders = NULL; // POST에서 사용
    string strResourceJSON = "";
    string filename = "";
    string filecontent = "";
    stringstream ss_postjson("");
    stringstream ss_urlPost("");
    CURLcode res;
    // struct stat file_info;
    curl_off_t speed_upload, total_time;
    long statLong = 0;
    FILE *fd_data = NULL;
    FILE *fd_header = NULL;
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    fd_data = fopen("/root/test/Week8/Week8/bin/post_data.txt", "w+");
    fd_header = fopen("/root/test/Week8/Week8/bin/post_header.txt", "w+");

    // file size 얻기 위해 사용
    // if(fstat(fileno(fd_send), &file_info) != 0)
    //     return false;

    // context 객체 생성
    curl = curl_easy_init();
    if(!curl) {
        cerr << "curl_easy_init() failed" << endl;
        curl_formfree(formpost);
        curl_global_cleanup();
        return false;
    }
    curl_global_init(CURL_GLOBAL_ALL);

    // responseHeaders = curl_slist_append(responseHeaders, "Content-Type: multipart/form-data");
    responseHeaders = curl_slist_append(responseHeaders, "Expect: ");

    curl_formadd(&formpost, &lastptr, 
            CURLFORM_COPYNAME, "sendfile", 
            CURLFORM_FILE, "/root/test/Week8/Week8/file_to_upload/test.txt", 
            CURLFORM_CONTENTTYPE, "text/plain",
            CURLFORM_END);

    curl_formadd(&formpost, &lastptr, 
            CURLFORM_COPYNAME, "sendfile2", 
            CURLFORM_FILE, "/root/test/Week8/Week8/file_to_upload/test2.txt", 
            CURLFORM_CONTENTTYPE, "text/plain",
            CURLFORM_END);

    curl_formadd(&formpost, &lastptr, 
            CURLFORM_COPYNAME, "sendjson", 
            CURLFORM_FILE, "/root/test/Week8/Week8/file_to_upload/test.json", 
            CURLFORM_CONTENTTYPE, "application/json",
            CURLFORM_END);

    // json 생성

    // METHOD : POST
    if(curl) { 
        curl_easy_setopt(curl, CURLOPT_URL, urlPost.c_str());

        // 업로드 하기 위해
        // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        // curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, responseHeaders);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

        // curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strResourceJSON.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        //결과 기록
        // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        // curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd_data);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, fd_header);

        res = curl_easy_perform(curl);

        cout << "------- POST " << endl;

        if(res != CURLE_OK) {
            cerr << "CURL_EASY_PERFORM() FAILED : " << curl_easy_strerror(res) << endl;
            return false;
        } else {
            res = curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &statLong);
            cout << ">> HTTP POST response code : " << statLong << endl;

            curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

            // cout << "SPEED : " << (unsigned long)speed_upload << " bytes/sec during " << (unsigned long)(total_time / 1000000) << " seconds" << endl;
        }

        // cout << chunk.memory << endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(responseHeaders);
        curl_formfree(formpost);
        curl_global_cleanup();

        return true;
    }
    return false;
}

size_t CallBackFunction(void* ptr, size_t size, size_t nmemb, void* userp)
{
    // ptr : 전달받은 데이터
    // size : 데이터 item 하나의 크기 
    // nmemb : 데이터 item 개수
    // -> 실제 데이터 크기 = size * nmemb
    // userp : CURLOPT_WRITEDATA 로 넘겨 받은 값
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        cout << "NOT ENOUGH MEMORY (relloc returned NULL)" << endl;
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}
