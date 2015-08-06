#pragma once

// source: http://www.gamedev.net/topic/651199-unique-ptr-and-emplace-back/
//std::make_unique implementation (it was forgotten in the C++11 standard, and will be added later).
//Once it's added, I can just remove this from here.

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
}

template<class T>
std::unique_ptr<T> make_unique()
{
    return std::unique_ptr<T>( new T() );
}
