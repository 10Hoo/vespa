// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <util/timer.h>
#include <httpclient/httpclient.h>
#include <util/filereader.h>
#include <util/clientstatus.h>
#include "client.h"
#include "fbench.h"
#include <cstring>
#include <cmath>
#include <csignal>

sig_atomic_t exitSignal = 0;

FBench::FBench()
    : _clients(),
      _ignoreCount(0),
      _cycle(0),
      _filenamePattern(NULL),
      _outputPattern(NULL),
      _byteLimit(0),
      _restartLimit(0),
      _maxLineSize(0),
      _keepAlive(true),
      _usePostMode(false),
      _headerBenchmarkdataCoverage(false),
      _seconds(60),
      _singleQueryFile(false)
{
}

FBench::~FBench()
{
    _clients.clear();
    free(_filenamePattern);
    free(_outputPattern);
}

void
FBench::InitBenchmark(int numClients, int ignoreCount, int cycle,
                      const char *filenamePattern, const char *outputPattern,
                      int byteLimit, int restartLimit, int maxLineSize,
                      bool keepAlive, bool headerBenchmarkdataCoverage, int seconds,
                      bool singleQueryFile, const std::string & queryStringToAppend, const std::string & extraHeaders,
                      const std::string &authority, bool postMode)
{
    _clients.resize(numClients);
    _ignoreCount     = ignoreCount;
    _cycle           = cycle;

    free(_filenamePattern);
    _filenamePattern = strdup(filenamePattern);
    free(_outputPattern);
    _outputPattern   = (outputPattern == NULL) ?
                       NULL : strdup(outputPattern);
    _queryStringToAppend = queryStringToAppend;
    _extraHeaders    = extraHeaders;
    _authority       = authority;
    _byteLimit       = byteLimit;
    _restartLimit    = restartLimit;
    _maxLineSize     = maxLineSize;
    _keepAlive       = keepAlive;
    _usePostMode     = postMode;
    _headerBenchmarkdataCoverage = headerBenchmarkdataCoverage;
    _seconds = seconds;
    _singleQueryFile = singleQueryFile;
}

void
FBench::CreateClients()
{
    int spread = (_cycle > 1) ? _cycle : 1;

    int i(0);
    for(auto & client : _clients) {
        uint64_t off_beg = 0;
        uint64_t off_end = 0;
        if (_singleQueryFile) {
            off_beg = _queryfileOffset[i];
            off_end = _queryfileOffset[i+1];
        }
        client = std::make_unique<Client>(
            new ClientArguments(i, _clients.size(), _filenamePattern,
                                _outputPattern, _hostnames[i % _hostnames.size()].c_str(),
                                _ports[i % _ports.size()], _cycle,
                                random() % spread, _ignoreCount,
                                _byteLimit, _restartLimit, _maxLineSize,
                                _keepAlive, _headerBenchmarkdataCoverage,
                                off_beg, off_end,
                                _singleQueryFile, _queryStringToAppend, _extraHeaders, _authority, _usePostMode));
        ++i;
    }
}

bool
FBench::ClientsDone()
{
    bool done(true);
    for (auto & client : _clients) {
        if ( ! client->done() ) {
            return false;
        }
    }
    return done;
}

void
FBench::StartClients()
{
    printf("Starting clients...\n");
    for (auto & client : _clients) {
        client->start();
    }
}

void
FBench::StopClients()
{
    printf("Stopping clients");
    for (auto & client : _clients) {
        client->stop();
    }
    printf("\nClients stopped.\n");
    for (auto & client : _clients) {
        client->join();
    }
    printf("\nClients Joined.\n");
}

void
FBench::PrintSummary()
{
    ClientStatus status;

    double maxRate    = 0;
    double actualRate = 0;

    int realNumClients = 0;
    
    int i = 0;
    for (auto & client : _clients) {
        if (client->GetStatus()._error) {
            printf("Client %d: %s => discarding client results.\n",
                   i, client->GetStatus()._errorMsg.c_str());
        } else {
            status.Merge(client->GetStatus());
            ++realNumClients;
        }
        ++i;
    }
    double avg = status.GetAverage();

    maxRate = (avg > 0) ? realNumClients * 1000.0 / avg : 0;
    actualRate = (status._realTime > 0) ?
                 realNumClients * 1000.0 * status._requestCnt / status._realTime : 0;

    double p25 = status.GetPercentile(25);
    double p50 = status.GetPercentile(50);
    double p75 = status.GetPercentile(75);
    double p90 = status.GetPercentile(90);
    double p95 = status.GetPercentile(95);
    double p99 = status.GetPercentile(99);

    if (_keepAlive) {
        printf("*** HTTP keep-alive statistics ***\n");
        printf("connection reuse count -- %zu\n", status._reuseCnt);
    }
    printf("***************** Benchmark Summary *****************\n");
    printf("clients:                %8ld\n", _clients.size());
    printf("ran for:                %8d seconds\n", _seconds);
    printf("cycle time:             %8d ms\n", _cycle);
    printf("lower response limit:   %8d bytes\n", _byteLimit);
    printf("skipped requests:       %8ld\n", status._skipCnt);
    printf("failed requests:        %8ld\n", status._failCnt);
    printf("successful requests:    %8ld\n", status._requestCnt);
    printf("cycles not held:        %8ld\n", status._overtimeCnt);
    printf("minimum response time:  %8.2f ms\n", status._minTime);
    printf("maximum response time:  %8.2f ms\n", status._maxTime);
    printf("average response time:  %8.2f ms\n", status.GetAverage());
    if (p25 > status._timetable.size() / status._timetableResolution - 1)
        printf("25 percentile:          %8.2f ms (approx)\n", p25);
    else         printf("25 percentile:          %8.2f ms\n", p25);
    if (p50 > status._timetable.size() / status._timetableResolution - 1)
        printf("50 percentile:          %8.2f ms (approx)\n", p50);
    else         printf("50 percentile:          %8.2f ms\n", p50);
    if (p75 > status._timetable.size() / status._timetableResolution - 1)
        printf("75 percentile:          %8.2f ms (approx)\n", p75);
    else         printf("75 percentile:          %8.2f ms\n", p75);
    if (p90 > status._timetable.size() / status._timetableResolution - 1)
        printf("90 percentile:          %8.2f ms (approx)\n", p90);
    else         printf("90 percentile:          %8.2f ms\n", p90);
    if (p95 > status._timetable.size() / status._timetableResolution - 1)
        printf("95 percentile:          %8.2f ms (approx)\n", p95);
    else         printf("95 percentile:          %8.2f ms\n", p95);
    if (p99 > status._timetable.size() / status._timetableResolution - 1)
        printf("99 percentile:          %8.2f ms (approx)\n", p99);
    else         printf("99 percentile:          %8.2f ms\n", p99);
    printf("actual query rate:      %8.2f Q/s\n", actualRate);
    printf("utilization:            %8.2f %%\n",
           (maxRate > 0) ? 100 * (actualRate / maxRate) : 0);
    printf("zero hit queries:       %8ld\n", status._zeroHitQueries);
    printf("http request status breakdown:\n");
    for (const auto& entry : status._requestStatusDistribution)
        printf("  %8u : %8u \n", entry.first, entry.second);
    
    fflush(stdout);
}

void
FBench::Usage()
{
    printf("usage: vespa-fbench [-H extraHeader] [-a queryStringToAppend ] [-n numClients] [-c cycleTime] [-l limit] [-i ignoreCount]\n");
    printf("              [-s seconds] [-q queryFilePattern] [-o outputFilePattern]\n");
    printf("              [-r restartLimit] [-m maxLineSize] [-k] <hostname> <port>\n\n");
    printf(" -H <str> : append extra header to each get request.\n");
    printf(" -A <str> : assign autority.  <str> should be hostname:port format. Overrides Host: header sent.\n");
    printf(" -P       : use POST for requests instead of GET.\n");
    printf(" -a <str> : append string to each query\n");
    printf(" -n <num> : run with <num> parallel clients [10]\n");
    printf(" -c <num> : each client will make a request each <num> milliseconds [1000]\n");
    printf("            ('-1' -> cycle time should be twice the response time)\n");
    printf(" -l <num> : minimum response size for successful requests [0]\n");
    printf(" -i <num> : do not log the <num> first results. -1 means no logging [0]\n");
    printf(" -s <num> : run the test for <num> seconds. -1 means forever [60]\n");
    printf(" -q <str> : pattern defining input query files ['query%%03d.txt']\n");
    printf("            (the pattern is used with sprintf to generate filenames)\n");
    printf(" -o <str> : save query results to output files with the given pattern\n");
    printf("            (default is not saving.)\n");
    printf(" -r <num> : number of times to re-use each query file. -1 means no limit [-1]\n");
    printf(" -m <num> : max line size in input query files [131072].\n");
    printf("            Can not be less than the minimum [1024].\n");
    printf(" -p <num> : print summary every <num> seconds.\n");
    printf(" -k       : disable HTTP keep-alive.\n");
    printf(" -y       : write data on coverage to output file.\n");
    printf(" -z       : use single query file to be distributed between clients.\n\n");
    printf(" <hostname> : the host you want to benchmark.\n");
    printf(" <port>     : the port to use when contacting the host.\n\n");
    printf("Several hostnames and ports can be listed\n");
    printf("This is distributed in round-robin manner to clients\n");
}

void
FBench::Exit()
{
    StopClients();
    printf("\n");
    PrintSummary();
    exit(0);
}

int
FBench::Main(int argc, char *argv[])
{
    // parameters with default values.
    int numClients  = 10;
    int cycleTime   = 1000;
    int byteLimit   = 0;
    int ignoreCount = 0;
    int seconds     = 60;
    int maxLineSize = 128 * 1024;
    const int minLineSize = 1024;

    const char *queryFilePattern  = "query%03d.txt";
    const char *outputFilePattern = NULL;
    std::string queryStringToAppend;
    std::string extraHeaders;

    int  restartLimit = -1;
    bool keepAlive    = true;
    bool headerBenchmarkdataCoverage = false;
    bool usePostMode = false;

    bool singleQueryFile = false;
    std::string authority;

    int  printInterval = 0;

    // parse options and override defaults.
    int         idx;
    char        opt;
    const char *arg;
    bool        optError;

    idx = 1;
    optError = false;
    while((opt = GetOpt(argc, argv, "H:A:a:n:c:l:i:s:q:o:r:m:p:kxyzP", arg, idx)) != -1) {
        switch(opt) {
        case 'A':
            authority = arg;
            break;
        case 'H':
            extraHeaders += std::string(arg) + "\r\n";
            if (strncmp(arg, "Host:", 5) == 0) {
                fprintf(stderr, "Do not override 'Host:' header, use -A option instead\n");
                return -1;
            }
            break;
        case 'a':
            queryStringToAppend = std::string(arg);
            break;
        case 'n':
            numClients = atoi(arg);
            break;
        case 'c':
            cycleTime = atoi(arg);
            break;
        case 'l':
            byteLimit = atoi(arg);
            break;
        case 'i':
            ignoreCount = atoi(arg);
            break;
        case 's':
            seconds = atoi(arg);
            break;
        case 'q':
            queryFilePattern = arg;
            break;
        case 'o':
            outputFilePattern = arg;
            break;
        case 'r':
            restartLimit = atoi(arg);
            break;
        case 'm':
            maxLineSize = atoi(arg);
            if (maxLineSize < minLineSize) {
                maxLineSize = minLineSize;
            }
            break;
        case 'P':
            usePostMode = true;
            break;
        case 'p':
            printInterval = atoi(arg);
            if (printInterval < 0)
                optError = true;
            break;
        case 'k':
            keepAlive = false;
            break;
        case 'x': 
            // consuming x for backwards compability. This turned on header benchmark data
            // but this is now always on. 
            break;
        case 'y':
            headerBenchmarkdataCoverage = true;
            break;
        case 'z':
            singleQueryFile = true;
            break;
        default:
            optError = true;
            break;
        }
    }

    if ( argc < (idx + 2) || optError) {
        Usage();
        return -1;
    }
    // Hostname/port must be in pair
    int args = (argc - idx);
    if (args % 2 != 0) {
        fprintf(stderr, "Not equal number of hostnames and ports\n");
        return -1;
    }

    short hosts = args / 2;

    for (int i=0; i<hosts; ++i)
    {
        _hostnames.push_back(std::string(argv[idx+2*i]));
        int port = atoi(argv[idx+2*i+1]);
        if (port == 0) {
            fprintf(stderr, "Not a valid port:\t%s\n", argv[idx+2*i+1]);
            return -1;
        }
        _ports.push_back(port);
    }

    // Find offset for each client if shared query file
    _queryfileOffset.push_back(0);
    if (singleQueryFile) {
        // Open file to find offsets, with pattern as if client 0
        char filename[1024];
        snprintf(filename, 1024, queryFilePattern, 0);
        queryFilePattern = filename;
        FileReader reader;
        if (!reader.Open(queryFilePattern)) {
            fprintf(stderr, "ERROR: could not open file '%s' [read mode]\n",
                    queryFilePattern);
            return -1;
        }

        uint64_t totalSize = reader.GetFileSize();
        uint64_t perClient = totalSize / numClients;

        for (int i=1; i<numClients; ++i) {
            /** Start each client with some offset, adjusted to next newline
             **/
            FileReader r;
            r.Open(queryFilePattern);
            uint64_t clientOffset = std::max(i*perClient, _queryfileOffset.back() );
            uint64_t newline = r.FindNextLine(clientOffset);
            _queryfileOffset.push_back(newline);
        }

        // Add pos to end of file
        _queryfileOffset.push_back(totalSize);


        // Print offset of clients
        /*
          printf("%6s%14s%15s", "Client", "Offset", "Bytes\n");
          for (unsigned int i =0; i< _queryfileOffset.size()-1; ++i)
          printf("%6d%14ld%14ld\n", i, _queryfileOffset[i], _queryfileOffset[i+1]-_queryfileOffset[i]);
        */
    }

    InitBenchmark(numClients, ignoreCount, cycleTime,
                  queryFilePattern, outputFilePattern,
                  byteLimit, restartLimit, maxLineSize,
                  keepAlive,
                  headerBenchmarkdataCoverage, seconds,
                  singleQueryFile, queryStringToAppend, extraHeaders,
                  authority, usePostMode);

    CreateClients();
    StartClients();

    if (seconds < 0) {
        unsigned int secondCount = 0;
        while (!ClientsDone()) {
            if (exitSignal) {
                _seconds = secondCount;
                Exit();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (printInterval != 0 && ++secondCount % printInterval == 0) {
                printf("\nRuntime: %d sec\n", secondCount);
                PrintSummary();
            }
        }
    } else if (seconds > 0) {
        // Timer to compansate for work load on PrintSummary()
        Timer sleepTimer;
        sleepTimer.SetMax(1000);

        for (;seconds > 0 && !ClientsDone(); seconds--) {
            if (exitSignal) {
                _seconds = _seconds - seconds;
                Exit();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(int(sleepTimer.GetRemaining())));
            sleepTimer.Start();

            if (seconds % 60 == 0) {
                printf("[dummydate]: PROGRESS: vespa-fbench: Seconds left %d\n", seconds);
            }

            if (printInterval != 0 && seconds % printInterval == 0) {
                printf("\nRuntime: %d sec\n", _seconds - seconds);
                PrintSummary();
            }

            sleepTimer.Stop();
        }
    }

    StopClients();
    PrintSummary();
    return 0;
}

void sighandler(int sig)
{
    if (sig == SIGINT) {
        exitSignal = 1;
    }
}

int
main(int argc, char** argv)
{

    struct sigaction act;

    act.sa_handler = sighandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGPIPE, &act, NULL);

    FBench myApp;
    return myApp.Main(argc, argv);
}
