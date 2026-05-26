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

struct Visitor final: public LibertyGroupVisitor {
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
	Group &stack_top() {
		if (stack_.empty()) {
			throw std::runtime_error("Group stack unexpectedly empty.");
		}
		return stack_.top();
	}
	
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

	static jsoncons::json valueAsJson(const LibertyAttrValue *value) {
		if (value->isString()) {
			if (value->stringValue() == "true") {
				return 1;
			}
			if (value->stringValue() == "false") {
				return 0;
			}
			return value->stringValue();
		}

		if (value->isFloat()) {
			auto [float_value, valid] = value->floatValue();
			if (valid) {
				return float_value;
			}
		}

		throw std::runtime_error("non-float or string attribute value");
	}
	
	void begin(const LibertyGroup *group, LibertyGroup *parent_group) override {
		(void) parent_group;
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

	void end(const LibertyGroup *group, LibertyGroup *parent_group) override {
		(void) parent_group;
		if (stack_top().subgroups_) {
			out_.end_array();
		}
		
		auto &attrs = stack_top().attrs_;
		if (attrs.size()) {
			for (const auto &[name, value]: attrs.object_range()) {
				out_.key(name);
				value.dump(out_);
			}
		}
		
		const auto &params = group->params();
		if (!params.empty()) {
			out_.key("names");
			out_.begin_array();
			for (auto param: params) {
				if (param->isFloat()) {
					auto [value, valid] = param->floatValue();
					if (valid) {
						out_.string_value(std::to_string(value));
					}
				} else {
					out_.string_value(param->stringValue());
				}
			}
			out_.end_array();
		}
		
		auto &variables = stack_top().variables_;
		for (auto &[name, value]: variables) {
			out_.key(name);
			out_.double_value(value);
		}
		
		auto &defines = stack_top().defines_;
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
		std::string_view name,
		std::string_view allowed_group_name,
		std::string_view valtype
	) {
		auto &defines = stack_top().defines_;
		string name_key(name);
		auto it = defines.find(name_key);
		if (it != defines.end() && it->second.allowed_group_name_.length() != 0) {
			it->second.allowed_group_name_ += "|"s + string(allowed_group_name);
			it->second.valtype_ = valtype;
		} else {
			defines[name_key] = Define {
				string(allowed_group_name),
				string(valtype)
			};
		}
	}

	void visitAttr(const LibertySimpleAttr *attr) override {
		const auto &value = attr->value();
		stack_top().attrs_[attr->name()] = valueAsJson(&value);
	}

	void visitAttr(const LibertyComplexAttr *attr) override {
		const auto &values = attr->values();
		if (attr->name() == "define_group") {
			if (values.empty()) {
				std::cerr << "WARNING: Malformed define_group on line " << attr->line() << ": no arguments provided." << std::endl;
				return;
			}
			if (values.size() < 2) {
				std::cerr << "WARNING: Malformed define_group on line " << attr->line() << ": less than two arguments provided." << std::endl;
				return;
			}
			
			std::string_view valtype = "undefined_valuetype";
			if (values.size() >= 3) {
				valtype = values[2]->stringValue();
			}
			makeOrAppendDefine(
				values[0]->stringValue(),
				values[1]->stringValue(),
				valtype
			);
		} else {
			jsoncons::json json_values(jsoncons::json_array_arg);
			for (auto value: values) {
				json_values.push_back(valueAsJson(value));
			}
			
			stack_top().attrs_[attr->name()] = json_values;
		}
	}

	void visitVariable(LibertyVariable *variable) override {
		stack_top().variables_.push_back({
				variable->variable(),
				variable->value()
		});
	}

	void visitDefine(LibertyDefine *define) override {
		auto groupTypeRaw = string(define->groupTypeRaw());
		makeOrAppendDefine(
			define->name(),
			groupTypeRaw.c_str(),
			attrTypeName(define->valueType())
		);
	}
};

STALibertyTranslator::STALibertyTranslator(
	string filename,
	std::ostream *out_stream,
	bool include_src_attributes
): filename_(filename) {
	visitor_ = std::unique_ptr<Visitor>(new Visitor(filename, out_stream, include_src_attributes));
	try {
		parseLibertyFile(filename_.c_str(), visitor_.get(), this);
	} catch (sta::Exception &e) {
		err_ = e.what();
	}
	
	log_stream_ = fdopen(STDERR_FILENO, "w");
}

STALibertyTranslator::~STALibertyTranslator() {}

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
