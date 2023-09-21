#include <Windows.h>
#include <iostream>
#include <string>
#include <intrin.h>

HANDLE ThreadStartEvent{ nullptr };

DWORD Thread( LPVOID context )
{
    const auto sem_handle{
        OpenSemaphoreW( SEMAPHORE_ALL_ACCESS,
                        FALSE,
                        L"Sem1" )
    };

    std::printf( "handle for \"Sem1\": 0x%8p\n",
                 sem_handle );

    if ( !SetEvent( ThreadStartEvent ) )
    {
        std::printf( "unable to set event: 0x%08x\n", GetLastError( ) );
        return 0;
    }

    WaitForSingleObject( sem_handle, INFINITE );

    std::printf( "wait satisfied\n" );

    CloseHandle( sem_handle );

    return 1;
}

int main_( )
{
    std::wcout << "[main thread] starting experiments"
            << "\n";


    const auto semaphore_handle{
        CreateSemaphoreW( nullptr,
                          0,
                          2,
                          L"Sem1" )
    };

    ThreadStartEvent = CreateEventW( nullptr, TRUE, FALSE, nullptr );

    std::printf( "semaphore handle: 0x%8p\n",
                 semaphore_handle );

    CreateThread( nullptr,
                  0,
                  Thread,
                  nullptr,
                  0,
                  nullptr );

    WaitForSingleObject( ThreadStartEvent, INFINITE );

    ReleaseSemaphore( semaphore_handle,
                      1,
                      nullptr );

    std::printf( "first semaphore release\n" );

    ReleaseSemaphore( semaphore_handle,
                      1,
                      nullptr );

    std::printf( "second semaphore release\n" );

    return 1;
}
