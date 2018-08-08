#include <httpserver/parser/to_json.h>
namespace httpserver {


void setJson(Json::Value& json, const vector<mor::DescField>& descs, const vector<shared_ptr<iField>>& fields){
    for(int i=0; i<fields.size(); i++){
        const mor::DescField& desc = descs[i];
        const auto& fi = fields[i];
        const std::type_index& type = *desc.typeinfo.get();

        if (type == typeid(unsigned short)){
            unsigned short* n = (unsigned short*) fi->value;
            json[desc.name] = *n;
        }

        else if(type == typeid(unsigned int) ){
            unsigned int n = *(unsigned int*) fi->value;
            json[desc.name] = n;
        }

        else if(type == typeid(unsigned long) ){
            Json::UInt64 n = (Json::UInt64) *(unsigned long*) fi->value;
            json[desc.name] = n;
        }

        else if(type == typeid(unsigned long long) ){
            unsigned long long n = *(unsigned long long*) fi->value;
            json[desc.name] = n;
        }

        else if( type == typeid(short) ){
            short n = *(short*) fi->value;
            json[desc.name] = n;
        }

        else if( type == typeid(int) ){
            int n = *(int*) fi->value;
            json[desc.name] = n;
        }

        else if(type == typeid(long) ){
            Json::Int64 n = (Json::Int64) *(long*) fi->value;
            json[desc.name] = n;
        }

        else if(type == typeid(long long) ){
            long long n = *(long long*) fi->value;
            json[desc.name] = n;
        }

        else if( type == typeid(double) ){
            double n = *(double*) fi->value;
            json[desc.name] = n;
        }

        else if(type == typeid(float) ){
            float n = *(float*) fi->value;
            json[desc.name] = n;
        }

        else if( type == typeid(bool) ){
            bool n = *(bool*) fi->value;
            json[desc.name] = n;
        }

        else{
            auto ref = desc.options.find("reference");
            if(ref != desc.options.end()){
                Json::Value obj;
                iEntity* ptr = (iEntity*) fi->value;
                setJson(obj, ptr->_get_desc_fields(), ptr->_get_fields());
                json[ref->second] = obj;
            }
            else
                json[desc.name] = fi->getValue(desc);
        }
    }
}


}
