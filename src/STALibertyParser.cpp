/*
	  liberty2json

		Copyright (C) 2025 Silimate Inc.

		This program is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version.

		This program is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "LibertyParser.hh"
#include "STALibertyParser.hpp" 
#include <iostream>
#include <stack>
#include <cstdlib>
using namespace sta;

class Visitor: public LibertyGroupVisitor {
	friend class STALibertyParser;
	json top;
	json::json_pointer current_group;
	
	static const char* groupTypeName(LibertyGroupType t) {
		switch (t) {
		case LibertyGroupType::library:
			return "library";
		case LibertyGroupType::cell:
			return "cell";
		case LibertyGroupType::pin:
			return "pin";
		case LibertyGroupType::timing:
			return "timing";
		default:
			return "unknown";
		}
	}
	
	static const char* attrTypeName(LibertyAttrType t) {
		switch (t) {
		case LibertyAttrType::attr_string:
			return "string";
		case LibertyAttrType::attr_int:
			return "int32";
		case LibertyAttrType::attr_double:
			return "float64";
		case LibertyAttrType::attr_boolean:
			return "boolean";
		default:
			return "unknown";
		}
	}
	
	Visitor() {
		top = {};
		current_group = json::json_pointer("");
	}
	
  void begin(LibertyGroup *group) {
		json::json_pointer groups_ptr = current_group;
		groups_ptr.push_back("groups");
		if (!top.contains(groups_ptr)) {
			top[groups_ptr] = json::array();
		}
		top.at(groups_ptr).push_back(json::object());
		groups_ptr.push_back(std::to_string(top.at(groups_ptr).size() - 1));
		groups_ptr.push_back(group->type());
		top[groups_ptr] = json::object();
		current_group = groups_ptr;
	}
  void end(LibertyGroup *group) {
		auto params = group->params();
		if (params) {
			json::array_t array;
			for (auto param: *group->params()) {
				if (param->isFloat()) {
					array.push_back(std::to_string(param->floatValue()));
				} else {
					array.push_back(param->stringValue());
				}
			}
			top.at(current_group)["names"] = array;
		}
		current_group.pop_back();
		current_group.pop_back();
		current_group.pop_back();
	}
	
	json::json_pointer prepare_define(const char *name) {
		json::json_pointer defines_ptr = current_group;
		defines_ptr.push_back("defines");
		if (!top.contains(defines_ptr)) {
			top[defines_ptr] = json::object();
		}

		json::json_pointer define_ptr = defines_ptr;
		define_ptr.push_back(name);
		if (!top.contains(define_ptr)) {
			top[define_ptr] = json::object();
		}

		return define_ptr;
	}
	
  void visitAttr(LibertyAttr *attr) {
		auto attr_ptr = current_group;
		attr_ptr.push_back(attr->name());
		if (attr->isComplex()) {
			if (strcmp(attr->name(), "define_group") == 0) {
				auto &values = *attr->values();
				
				auto define_ptr = prepare_define(values[0]->stringValue());
				std::string allowed_groups = values[1]->stringValue();
				if (top[define_ptr].contains("allowed_group_name")) {
					allowed_groups = std::string(top[define_ptr]["allowed_group_name"]) + "|" + allowed_groups;
				}
				top[define_ptr]["allowed_group_name"] = allowed_groups;
				
				if (values.size() < 3) {
					top[define_ptr]["valtype"] = "undefined_valuetype";
				} else {
					top[define_ptr]["valtype"] = values[2]->stringValue();
				}
			} else {
				json::array_t values;
				for (auto value: *attr->values()) {
					if (value->isFloat()) {
						values.push_back(value->floatValue());
					} else if (value->isString()) {
						values.push_back(value->stringValue());
					}
				}
				top[attr_ptr] = values;
			}
		} else {
			auto value = attr->firstValue();
			if (value->isFloat()) {
				top[attr_ptr] = value->floatValue();
			} else if (value->isString()) {
				if (strcmp(value->stringValue(), "true") == 0) {
					top[attr_ptr] = 1;
				} else if (strcmp(value->stringValue(), "false") == 0) {
					top[attr_ptr] = 0;
				} else {
					top[attr_ptr] = value->stringValue();
				}
			} else {
				std::cout << attr->name() << std::endl;
			}
		}
	}
  void visitVariable(LibertyVariable *variable) {
		top[current_group][variable->variable()] = variable->value();
	}
	void visitDefine(LibertyDefine *define) {
		auto define_ptr = prepare_define(define->name());
		std::string allowed_groups { define->groupTypeRaw() };
		if (top[define_ptr].contains("allowed_group_name")) {
			allowed_groups = std::string(top[define_ptr]["allowed_group_name"]) + "|" + allowed_groups;
		}
		top[define_ptr]["allowed_group_name"] = allowed_groups;
		top[define_ptr]["valtype"] = attrTypeName(define->valueType());
	}
  // Predicates to save parse structure after visits.
  bool save(LibertyGroup *group) { return true; }
  bool save(LibertyAttr *attr) { return true; }
  bool save(LibertyVariable *variable) { return true; }
};

STALibertyParser::STALibertyParser(std::string filename): filename(filename) {
	visitor = new Visitor();
	parseLibertyFile(filename.c_str(), visitor, this);
}

STALibertyParser::~STALibertyParser() {
	delete visitor;
}

int STALibertyParser::check() {
	// Out-of-line implementation for check
	return 0; 
}

std::string STALibertyParser::get_error_text() {
	// Out-of-line implementation for get_error_text
	return "";
}

nlohmann::json STALibertyParser::as_json() {
	json::object_t object;
	object["library"] = visitor->top["groups"][0]["library"];
	return object;
}

void STALibertyParser::to_json_file(std::string filename, bool indent) {
	std::ofstream file(filename);
	if (indent)
		file << as_json().dump(2);
	else
		file << as_json().dump();
	file.close();
}
