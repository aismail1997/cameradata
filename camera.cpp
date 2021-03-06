#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <iostream>
#include <curl/curl.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <boost/atomic.hpp>

/*
--myboundary
Content-Type: image/jpeg
Content-Length: 53933

*/
int producer_count = 0;
boost::atomic_int consumer_count (0);

boost::lockfree::spsc_queue<int, boost::lockfree::capacity<1024> > spsc_queue;

const int iterations = 10000000;

void producer(void)
{
    for (int i = 0; i != iterations; ++i) {
        int value = ++producer_count;
	
	//int value = 
        while (!spsc_queue.push(value));
    }
}

boost::atomic<bool> done (false);

void consumer(void)
{
    int value;
    while (!done) {
        while (spsc_queue.pop(value))
            ++consumer_count;
    }

    while (spsc_queue.pop(value))
        ++consumer_count;
}


 
struct FtpFile {
  const char *filename;
  FILE *stream;
};
 
static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
  struct FtpFile *out=(struct FtpFile *)stream;
  if(out && !out->stream) {
    /* open file for writing */ 
    out->stream=fopen(out->filename, "wb");
    if(!out->stream)
      return -1; /* failure, can't open file to write */ 
  }
  return fwrite(buffer, size, nmemb, out->stream);
}


int main(int argc, char* argv[])
{
CURL *curl;
  CURLcode res;

    using namespace std;

	struct FtpFile ftpfile={
    "mycam1.mpeg", /* name to store the file as if successful */ 
    NULL
  };
 
  curl_global_init(CURL_GLOBAL_DEFAULT);
 
  curl = curl_easy_init();
  if(curl) {

    curl_easy_setopt(curl, CURLOPT_URL, "http://root:georgep@143.215.207.165/axis-cgi/mjpg/video.cgi?streamprofile=vid1&duration=1");
 //curl_easy_setopt(curl, CURLOPT_URL, "http://root:georgep@143.215.207.165/axis-cgi/jpg/image.cgi?");
 
    /* Define our callback to get called when there's data to be written */ 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
    /* Set a pointer to our struct to pass to the callback */ 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
 
    /* Switch on full protocol/debug output */ 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

 if(ftpfile.stream)
    fclose(ftpfile.stream); /* close the local file */ 
 
  curl_global_cleanup();

  }



/*
  streampos size;
  char * memblock;

  ifstream file ("mycam.jpeg", ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    size = file.tellg();
    memblock = new char [size];
    file.seekg (0, ios::beg);
    file.read (memblock, size);
    file.close();

    cout << "the entire file content is in memory\n";

    //delete[] memblock;
  }
  else cout << "Unable to open file";

*/
 cout << "boost::lockfree::queue is ";
    if (!spsc_queue.is_lock_free())
        cout << "not ";
    cout << "lockfree" << endl;

    boost::thread producer_thread(producer);
    boost::thread consumer_thread(consumer);

    producer_thread.join();
    done = true;
    consumer_thread.join();

    cout << "produced " << producer_count << " objects." << endl;
    cout << "consumed " << consumer_count << " objects." << endl;

//delete[] memblock;
}
