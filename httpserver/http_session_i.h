#ifndef HTTP_SESSION_I_H
#define HTTP_SESSION_I_H
#include <unordered_map>
#include <thread>
#include <boost/any.hpp>

namespace httpserver {
class dynamic_request;

class http_session_i
{

    std::unordered_map<std::string, boost::any> data_;
protected:
public :
    virtual ~http_session_i();

    virtual dynamic_request& request() = 0;

    // Return a object in map
    template< class type>
    type& get_data(const char* key){
        auto&& itr = data_.find(key);
        if(itr != data_.end())
        {
            boost::any& a = itr->second;
            return boost::any_cast<type&>(a);
        }
        throw std::runtime_error(std::string("http_session::data: ")+key+" not found");
    }

    // Return a object in map, return default id not fond
    template< class type>
    type& get_data(const char* key, const type& defaultValue){
        auto&& itr = data_.find(key);
        if(itr != data_.end())
        {
            boost::any& a = itr->second;
            return boost::any_cast<type&>(a);
        }
        return const_cast<type&>(defaultValue);
    }

    // Put a object in map
    template< class type>
    void set_data(const char* key, type&& obj){
        data_[key] = obj;
    }
};

extern std::unordered_map<std::thread::id, http_session_i*> map_http_session;

http_session_i* get_http_session();

}
#endif // HTTP_SESSION_I_H
