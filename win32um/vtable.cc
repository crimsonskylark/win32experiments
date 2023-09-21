#include <cstdint>
#include <cstdio>

class parent_t
{
    virtual const char *get_name( ) = 0;
    virtual int get_age( ) = 0;
};

class child_t : parent_t
{
public:
    const char *get_name( ) override { return "name"; }

    int get_age( ) override { return 20; }
};

int main( )
{
    const auto child{ new child_t };

    std::printf( "%s\n",
                 child->get_name ( ) );

    // same thing as

    using get_name_t = const char *( __thiscall * )( child_t *instance );
    std::printf( "%s\n",
                 reinterpret_cast< get_name_t >( **reinterpret_cast< uintptr_t*** >( child ) )( child ) );
}
