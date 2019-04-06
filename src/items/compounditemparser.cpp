/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "compounditemparser.h"

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

CompoundItemParser::CompoundItemParser (const nlohmann::json& item_definition)
 : ItemParserBase (item_definition)
{
    assert (type_ == "compound");

    if (item_definition.find("field_specification") == item_definition.end())
        throw runtime_error ("compound item '"+name_+"' parsing without field specification");

    const json& field_specification = item_definition.at("field_specification");

    if (!field_specification.is_object())
        throw runtime_error ("parsing compound item '"+name_+"' field specification is not an object");

    //field_specification_name_ = field_specification.at("name");

    field_specification_.reset(ItemParserBase::createItemParser(field_specification));
    assert (field_specification_);

    if (item_definition.find("items") == item_definition.end())
        throw runtime_error ("parsing compound item '"+name_+"' without items");

    const json& items = item_definition.at("items");

    if (!items.is_array())
        throw runtime_error ("parsing compound item '"+name_+"' field specification is not an array");

    std::string item_name;
    ItemParserBase* item {nullptr};

    for (const json& data_item_it : items)
    {
        item_name = data_item_it.at("name");
        item = ItemParserBase::createItemParser(data_item_it);
        assert (item);
        items_.push_back(std::unique_ptr<ItemParserBase>{item});
    }
}

size_t CompoundItemParser::parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug)
{
    if (debug)
        loginf << "parsing compound item '" << name_ << "' with " << items_.size() << " items" << logendl;

    size_t parsed_bytes {0};

    if (debug)
        loginf << "parsing compound item '"+name_+"' field specification" << logendl;

    parsed_bytes = field_specification_->parseItem(data, index+parsed_bytes, size, parsed_bytes, target, debug);

    if (debug)
        loginf << "parsing compound item '"+name_+"' data items" << logendl;

    for (auto& data_item_it : items_)
    {
        if (debug)
            loginf << "parsing compound item '" << name_ << "' data item '" << data_item_it->name() << "' index "
                   << index+parsed_bytes << logendl;

        parsed_bytes += data_item_it->parseItem(data, index+parsed_bytes, size, parsed_bytes, target, debug);
    }

    return parsed_bytes;
}

}
