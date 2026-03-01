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
#include "STALibertyTranslator.hpp" 

#include "Sta.hh"

#include <jsoncons/basic_json.hpp>
#include <jsoncons/json_encoder.hpp>

#include <iostream>
#include <stack>
#include <cstdlib>
#include <map>

using namespace sta;
using namespace std::literals::string_literals;

class Visitor: public LibertyGroupVisitor {
	friend class STALibertyTranslator;
	std::filesystem::path src_;
	jsoncons::json_stream_encoder out_;
	bool include_src_attributes_;
	
	struct Variable {
		string name_;
		float value_;
	};

	struct Define {
		string allowed_group_name_;
		string valtype_;
	};
	
	struct Group {
		string type_;
		bool subgroups_;
		std::vector<Variable> variables_;
		std::map<string, Define> defines_;
		jsoncons::json attrs_;
	};
	
	std::stack<Group> stack_;
	
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
	
	Visitor(
		std::filesystem::path src,
		std::ostream *out_stream,
		bool include_src_attributes
	):
		src_(src),
		out_(*out_stream),
		include_src_attributes_(include_src_attributes)
	{}
	
	~Visitor() {}
	
	void begin(LibertyGroup *group) {
		if (stack_.size() && !stack_.top().subgroups_) {
			stack_.top().subgroups_ = true;
			out_.key("groups");
			out_.begin_array();
		}
		stack_.push(Group {group->type()});
		out_.begin_object();
		out_.key(group->type());
		out_.begin_object();
	}

	void end(LibertyGroup *group) {
		if (stack_.size() && stack_.top().subgroups_) {
			out_.end_array();
		}
		
		auto &attrs = stack_.top().attrs_;
		if (attrs.size()) {
			for (const auto &[name, value]: attrs.object_range()) {
				out_.key(name);
				value.dump(out_);
			}
		}
		
		auto params = group->params();
		if (params) {
			out_.key("names");
			out_.begin_array();
			for (auto param: *group->params()) {
				if (param->isFloat()) {
					out_.string_value(std::to_string(param->floatValue()));
				} else {
					out_.string_value(param->stringValue());
				}
			}
			out_.end_array();
		}
		
		auto &variables = stack_.top().variables_;
		for (auto &[name, value]: variables) {
			out_.key(name);
			out_.double_value(value);
		}
		
		auto &defines = stack_.top().defines_;
		if (defines.size()) {
			out_.key("defines");
			out_.begin_object();
			for (auto &[name, define]: defines) {
				out_.key(name);
				out_.begin_object();
				out_.key("allowed_group_name");
				out_.string_value(define.allowed_group_name_);
				out_.key("valtype");
				out_.string_value(define.valtype_);
				out_.end_object();
			}
			out_.end_object();
		}
		
		if (include_src_attributes_) {
			out_.key("src");
			string src_attr = src_.string() + ":" + std::to_string(group->line());
			out_.string_value(src_attr);
		}
		out_.end_object();
		out_.end_object();
		stack_.pop();
		out_.flush();
	}

	void makeOrAppendDefine(
		const char *name,
		const char *allowed_group_name,
		const char *valtype
	) {
		auto &defines = stack_.top().defines_;
		auto it = defines.find(name);
		if (it != defines.end() && it->second.allowed_group_name_.length() != 0) {
			it->second.allowed_group_name_ += "|"s + string(allowed_group_name);
			it->second.valtype_ = valtype;
		} else {
			defines[name] = Define {
				allowed_group_name,
				valtype
			};
		}
	}

	void visitAttr(LibertyAttr *attr) {
		if (attr->isComplex()) {
			if (strcmp(attr->name(), "define_group") == 0) {
				auto &values = *attr->values();
				
				const char *valtype = "undefined_valuetype";
				if (values.size() >= 3) {
					valtype = values[2]->stringValue();
				}
				makeOrAppendDefine(
					values[0]->stringValue(),
					values[1]->stringValue(),
					valtype
				);
			} else {
				jsoncons::json values(jsoncons::json_array_arg);
				for (auto value: *attr->values()) {
					if (value->isFloat()) {
						values.push_back(value->floatValue());
					} else if (value->isString()) {
						values.push_back(value->stringValue());
					}
				}
				stack_.top().attrs_[attr->name()] = values;
			}
		} else {
			auto value = attr->firstValue();
			if (value->isFloat()) {
				stack_.top().attrs_[attr->name()] = value->floatValue();
			} else if (value->isString()) {
				if (strcmp(value->stringValue(), "true") == 0) {
					stack_.top().attrs_[attr->name()] = 1;
				} else if (strcmp(value->stringValue(), "false") == 0) {
					stack_.top().attrs_[attr->name()] = 0;
				} else {
					stack_.top().attrs_[attr->name()] = value->stringValue();
				}
			} else {
				throw std::runtime_error(string("Unhandled attribute ") + attr->name());
			}
		}
	}
	void visitVariable(LibertyVariable *variable) {
		stack_.top().variables_.push_back({
				variable->variable(),
				variable->value()
		});
	}

	void visitDefine(LibertyDefine *define) {
		auto groupTypeRaw = string(define->groupTypeRaw());
		makeOrAppendDefine(
			define->name(),
			groupTypeRaw.c_str(),
			attrTypeName(define->valueType())
		);
	}
	// Predicates to save parse structure after visits.
	bool save(LibertyGroup *group) { return false; }
	bool save(LibertyAttr *attr) { return false; }
	bool save(LibertyVariable *variable) { return false; }
};

STALibertyTranslator::STALibertyTranslator(
	string filename,
	std::ostream *out_stream,
	bool include_src_attributes
): filename_(filename) {
	visitor_ = new Visitor(filename, out_stream, include_src_attributes);
	try {
		parseLibertyFile(filename_.c_str(), visitor_, this);
	} catch (sta::Exception &e) {
		err_ = e.what();
	}
	
	log_stream_ = fdopen(STDERR_FILENO, "w");
}

STALibertyTranslator::~STALibertyTranslator() {
	delete visitor_;
}

int STALibertyTranslator::check() {
	return err_.has_value();
}

string STALibertyTranslator::get_error_text() {
	return err_.value();
}

size_t STALibertyTranslator::printConsole(const char *buffer, size_t length) {
	std::cerr << std::string_view(buffer, length) << std::endl;
	return length;
}
