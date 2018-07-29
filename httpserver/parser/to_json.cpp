#include <httpserver/parser/to_json.h>
namespace httpserver {


void setJson(Json::Value& json, const vector<mor::DescField>& descs, const vector<shared_ptr<iField>>& fields){
    for(int i=0; i<fields.size(); i++){
        const mor::DescField& desc = descs[i];
        const auto& fi = fields[i];
        const std::type_index& type = *desc.typeinfo.get();

        if (type == typeid(unsigned short))
            json[desc.name] = *(unsigned short*) fi->value;

        else if(type == typeid(unsigned int) )
            json[desc.name] = *(unsigned int*) fi->value;

        else if(type == typeid(unsigned long) )
            json[desc.name] = (Json::UInt64) *(unsigned long*) fi->value;

        else if(type == typeid(unsigned long long) )
            json[desc.name] = *(unsigned long long*) fi->value;

        else if( type == typeid(short) )
            json[desc.name] = *(short*) fi->value;

        else if( type == typeid(int) )
            json[desc.name] = *(int*) fi->value;

        else if(type == typeid(long) )
            json[desc.name] = (Json::Int64) *(long*) fi->value;

        else if(type == typeid(long long) )
            json[desc.name] = *(long long*) fi->value;

        else if( type == typeid(double) )
            json[desc.name] = *(double*) fi->value;

        else if(type == typeid(float) )
            json[desc.name] = *(float*) fi->value;

        else if( type == typeid(bool) )
            json[desc.name] = *(bool*) fi->value;

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
