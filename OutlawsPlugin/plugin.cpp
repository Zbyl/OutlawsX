


#define OUTLAWSX_API __declspec(dllexport)

extern "C" {

OUTLAWSX_API int func(int a)
{
    return a + 1;
}

} // extern "C"
