# PatternMatching

**Motivation:** there's no pattern matching in C++, but sometimes it can make your code
more clean, than without it. Consider this example:

```cpp
//
// You need to call Java fucntion from C++.
// Usually you do this way:
//

int JavaCallHelper(int elem)
{
    return jni->CallIntFunction( elem );
}

float JavaCallHelper(float elem)
{
    return jni->CallFloatFunction( elem );
}

template<typename _Type>
auto JavaCall(_Type elem)
{
    return JavaCallHelper(elem);
}
```

With pattern matching you can do such beautiful thing:
```cpp
template<typename _Type>
auto JavaCall(_Type elem)
{
    // Elements are passed as tuple in order 
    // to support multiple arguments
    return match::Match(
        std::make_tuple( elem ),
        [](int elem)   { return jni->CallIntFunction( elem ); },
        [](float elem) { return jni->CallFloatFunction( elem ); }
    );
}
```

### Advantages

- Header only library
- C++14 supported
- Dispatch is a compile-time process => no runtime delay
