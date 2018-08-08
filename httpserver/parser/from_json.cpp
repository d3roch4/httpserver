#include <httpserver/parser/from_json.h>
#include <mor/ientity.h>

namespace httpserver {

    void preencheReferencia(mor::DescField& desc, mor::iField* field, const Json::Value& json);

    response& operator << (response&& resp, const Json::Value& json)
    {
        resp.body() += json.toStyledString();
    }

    void from_json(std::vector<mor::DescField> &vecDescs, std::vector<std::shared_ptr<mor::iField> > &vecFields, const Json::Value& json)
    {
        int i = 0;
        const Json::Value::Members& members = json.getMemberNames();
        for (const Json::Value& value : json) {
            for (int j = 0; j < vecDescs.size(); j++) {
                auto ref = vecDescs[j].options.find("reference");
                if(ref != vecDescs[j].options.end() && ref->second==members[i])
                    preencheReferencia(vecDescs[j], vecFields[j].get(), value);
                else if (vecDescs[j].name == members[i]) {
                    switch (value.type()) {
                        case Json::ValueType::intValue: ///< signed integer value
                            vecFields[j]->setValue(std::to_string(value.asInt64()).c_str(), vecDescs[j]);
                            break;
                        case Json::ValueType::uintValue: ///< unsigned integer value
                            vecFields[j]->setValue(std::to_string(value.asUInt64()).c_str(), vecDescs[j]);
                            break;
                        case Json::ValueType::realValue: ///< double value
                            vecFields[j]->setValue(std::to_string(value.asDouble()).c_str(), vecDescs[j]);
                            break;
                        case Json::ValueType::stringValue: ///< UTF-8 string value
                            vecFields[j]->setValue(value.asCString(), vecDescs[j]);
                            break;
                        case Json::ValueType::booleanValue: ///< bool value
                            vecFields[j]->setValue(std::to_string(value.asBool()).c_str(), vecDescs[j]);
                            break;
                        case Json::ValueType::objectValue:
                        case Json::ValueType::arrayValue: ///< array value (ordered list)
                        case Json::ValueType::nullValue: ///< 'null' value
                        default:
                            //logger("from_json: value '"+field->name+"' with type not reconized!");
                            break;
                    }
                }
            }
            i++;
        }
    }

    void preencheReferencia(mor::DescField& desc, mor::iField* field, const Json::Value& json)
    {
        mor::iEntity* ptr = (mor::iEntity*) field->value;
        auto&& vecDesc = ptr->_get_desc_fields();
        auto&& vecField = ptr->_get_fields();
        from_json(vecDesc, vecField, json);
    }
}
