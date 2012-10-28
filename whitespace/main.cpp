#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <functional>
#include <regex>
#include <algorithm>

#define DEBUG

typedef long long base_unit_type;

struct state;

struct wsobject
{
	enum type
	{
		operation=0,
		label=1
	};

	type objecttype;
	long long labelvalue,length;
	std::function<void(state&)> function;

	wsobject(const wsobject& obj)
	{
		objecttype=obj.objecttype;
		labelvalue=obj.labelvalue;
		length=obj.length;
		function=obj.function;
		return;
	}

	wsobject(std::function<void(state&)> opfunc)
	{
		objecttype=type::operation;
		function=opfunc;
		labelvalue=0;
		length=0;
		return;
	}

	wsobject(long long lb,long long len)
	{
		objecttype=type::label;
		labelvalue=lb;
		length=len;
		return;
	}
};

struct state
{
	std::deque<base_unit_type> data;
	std::deque<std::vector<wsobject>::iterator> calls;
	std::vector<base_unit_type> heap;
	std::vector<wsobject> instructions;
	std::vector<wsobject>::iterator instptr;
};

wsobject stackcontrol(std::string::const_iterator& it,char space,char tab,char newline)
{
	if(*++it==space){
#ifdef DEBUG
		std::cout<<"[s][s]"<<std::endl;
#endif
		base_unit_type n=0,i;
		auto cur=it;
		for(;*cur!=newline;++cur) continue;
		auto end=cur;
		for(i=0,--cur;cur!=it;++i,--cur) n|=(*cur==tab?1:0)<<i;
		it=end;
		return wsobject(
			[n](state& stateobj)
			{
				stateobj.data.push_back(n);
				++stateobj.instptr;
			});
	}else if(*it==tab){
		if(*++it==space){
#ifdef DEBUG
			std::cout<<"[s][t][s]"<<std::endl;
#endif
			base_unit_type n=0,i;
			auto cur=it;
			for(;*cur!=newline;++cur) continue;
			auto end=cur;
			for(i=0,--cur;cur!=it;++i,--cur) n|=(*cur==tab?1:0)<<i;
			it=end;
			return wsobject(
				[n](state& stateobj)
				{
					stateobj.data.push_back(stateobj.data[n]);
					++stateobj.instptr;
				});
		}else if(*it==newline){
#ifdef DEBUG
			std::cout<<"[s][t][n]"<<std::endl;
#endif
			base_unit_type n=0,i;
			auto cur=it;
			for(;*cur!=newline;++cur) continue;
			auto end=cur;
			for(i=0,--cur;cur!=it;++i,--cur) n|=(*cur==tab?1:0)<<i;
			return wsobject(
				[n](state& stateobj)
				{
					auto top=stateobj.data.back();
					for(int i=0;i<n;++i) stateobj.data.pop_back();
					stateobj.data.push_back(top);
					++stateobj.instptr;
				});
		}
	}else if(*it==newline){
		if(*++it==space){
#ifdef DEBUG
			std::cout<<"[s][n][s]"<<std::endl;
#endif
			return wsobject(
				[](state& stateobj)
				{
					stateobj.data.push_back(stateobj.data.back());
					++stateobj.instptr;
				});
		}else if(*it==tab){
#ifdef DEBUG
			std::cout<<"[s][n][t]"<<std::endl;
#endif
			return wsobject(
				[](state& stateobj)
				{
					auto a=stateobj.data.back();
					stateobj.data.pop_back();
					auto b=stateobj.data.back();
					stateobj.data.pop_back();
					stateobj.data.push_back(a);
					stateobj.data.push_back(b);
					++stateobj.instptr;
				});
		}else if(*it==newline){
#ifdef DEBUG
			std::cout<<"[s][n][n]"<<std::endl;
#endif
			return wsobject(
				[](state& stateobj)
				{
					stateobj.data.pop_back();
					++stateobj.instptr;
				});
		}
	}
}

wsobject arithmetic(std::string::const_iterator& it,char space,char tab,char newline)
{
	if(*++it==space){
		if(*++it==space){
#ifdef DEBUG
			std::cout<<"[t][s][s][s]"<<std::endl;
#endif
			return wsobject(
			    [](state& stateobj)
				{
					auto a=stateobj.data.back();
					stateobj.data.pop_back();
					auto b=stateobj.data.back();
					stateobj.data.pop_back();
					stateobj.data.push_back(a+b);
					++stateobj.instptr;
				});
		}else if(*it==tab){
#ifdef DEBUG
			std::cout<<"[t][s][s][t]"<<std::endl;
#endif
			return wsobject(
			    [](state& stateobj)
				{
					auto a=stateobj.data.back();
					stateobj.data.pop_back();
					auto b=stateobj.data.back();
					stateobj.data.pop_back();
					stateobj.data.push_back(a-b);
					++stateobj.instptr;
				});
		}else if(*it==newline){
#ifdef DEBUG
			std::cout<<"[t][s][s][n]"<<std::endl;
#endif
			return wsobject(
			    [](state& stateobj)
				{
					auto a=stateobj.data.back();
					stateobj.data.pop_back();
					auto b=stateobj.data.back();
					stateobj.data.pop_back();
					stateobj.data.push_back(a*b);
					++stateobj.instptr;
				});
		}
	}else if(*it==tab){
		if(*++it==space){
#ifdef DEBUG
			std::cout<<"[t][s][t][s]"<<std::endl;
#endif
			return wsobject(
			    [](state& stateobj)
				{
					auto a=stateobj.data.back();
					stateobj.data.pop_back();
					auto b=stateobj.data.back();
					stateobj.data.pop_back();
					stateobj.data.push_back(a/b);
					++stateobj.instptr;
				});
		}else if(*it==tab){
#ifdef DEBUG
			std::cout<<"[t][s][t][t]"<<std::endl;
#endif
			return wsobject(
			    [](state& stateobj)
				{
					auto a=stateobj.data.back();
					stateobj.data.pop_back();
					auto b=stateobj.data.back();
					stateobj.data.pop_back();
					stateobj.data.push_back(a%b);
					++stateobj.instptr;
				});
		}
	}
}

wsobject heapaccess(std::string::const_iterator& it,char space,char tab,char newline)
{
	if(*++it==space){
#ifdef DEBUG
			std::cout<<"[t][t][s]"<<std::endl;
#endif
			return wsobject(
			    [](state& stateobj)
				{
					auto a=stateobj.data.back();
					stateobj.data.pop_back();
					auto b=stateobj.data.back();
					stateobj.data.pop_back();
					if(b>=stateobj.heap.size()) stateobj.heap.resize((b+1)*2);
					stateobj.heap[b]=a;
					++stateobj.instptr;
				});
	}else if(*it==tab){
#ifdef DEBUG
			std::cout<<"[t][t][t]"<<std::endl;
#endif
			return wsobject(
			    [](state& stateobj)
				{
					auto a=stateobj.data.back();
					stateobj.data.pop_back();
					stateobj.data.push_back(stateobj.heap[a]);
					++stateobj.instptr;
				});
	}
}

wsobject flowcontrol(std::string::const_iterator& it,char space,char tab,char newline)
{
	if(*++it==space){
		if(*++it==space){
#ifdef DEBUG
			std::cout<<"[n][s][s]"<<std::endl;
#endif
			base_unit_type n=0,i;
			auto cur=it;
			for(;*++cur!=newline;) continue;
			auto end=cur;
			for(i=0,--cur;cur!=it;++i,--cur) n|=(*cur==tab?1:0)<<i;
			it=end;
			return wsobject(n,i);
		}else if(*it==tab){
#ifdef DEBUG
			std::cout<<"[n][s][t]"<<std::endl;
#endif
			base_unit_type n=0,i;
			auto cur=it;
			for(;*++cur!=newline;) continue;
			auto end=cur;
			for(i=0,--cur;cur!=it;++i,--cur) n|=(*cur==tab?1:0)<<i;
			it=end;
			return wsobject(
				[n,i](state& stateobj)
				{
					auto _n=n,_i=i;
					stateobj.calls.push_back(++stateobj.instptr);
					stateobj.instptr=std::find_if(
						stateobj.instructions.begin(),
						stateobj.instructions.end(),
						[_n,_i](const wsobject& obj){return obj.objecttype==wsobject::type::label&&obj.labelvalue==_n&&obj.length==_i;});
				});
		}else if(*it==newline){
#ifdef DEBUG
			std::cout<<"[n][s][n]"<<std::endl;
#endif
			base_unit_type n=0,i;
			auto cur=it;
			for(;*++cur!=newline;) continue;
			auto end=cur;
			for(i=0,--cur;cur!=it;++i,--cur) n|=(*cur==tab?1:0)<<i;
			it=end;
			return wsobject(
				[n,i](state& stateobj)
				{
					auto _n=n,_i=i;
					stateobj.instptr=std::find_if(
						stateobj.instructions.begin(),
						stateobj.instructions.end(),
						[_n,_i](const wsobject& obj){return obj.objecttype==wsobject::type::label&&obj.labelvalue==_n&&obj.length==_i;});
				});
		}
	}else if(*it==tab){
		if(*++it==space){
#ifdef DEBUG
			std::cout<<"[n][t][s]"<<std::endl;
#endif
			base_unit_type n=0,i;
			auto cur=it;
			for(;*++cur!=newline;) continue;
			auto end=cur;
			for(i=0,--cur;cur!=it;++i,--cur) n|=(*cur==tab?1:0)<<i;
			it=end;
			return wsobject(
			    [n,i](state& stateobj)
				{
					auto _n=n,_i=i;
					if(!stateobj.data.back()){
						stateobj.instptr=std::find_if(
							stateobj.instructions.begin(),
							stateobj.instructions.end(),
							[_n,_i](const wsobject& obj){return obj.objecttype==wsobject::type::label&&obj.labelvalue==_n&&obj.length==_i;});
					}else ++stateobj.instptr;
					stateobj.data.pop_back();
				});
		}else if(*it==tab){
#ifdef DEBUG
			std::cout<<"[n][t][t]"<<std::endl;
#endif
			base_unit_type n=0,i;
			auto cur=it;
			for(;*++cur!=newline;) continue;
			auto end=cur;
			for(i=0,--cur;cur!=it;++i,--cur) n|=(*cur==tab?1:0)<<i;
			it=end;
			return wsobject(
			    [n,i](state& stateobj)
				{
					auto _n=n,_i=i;
					if(stateobj.data.back()<0){
						stateobj.instptr=std::find_if(
							stateobj.instructions.begin(),
							stateobj.instructions.end(),
							[_n,_i](const wsobject& obj){return obj.objecttype==wsobject::type::label&&obj.labelvalue==_n&&obj.length==_i;});
					}else ++stateobj.instptr;
					stateobj.data.pop_back();
				});
		}else if(*it==newline){
#ifdef DEBUG
			std::cout<<"[n][t][n]"<<std::endl;
#endif
			return wsobject(
				[](state& stateobj)
				{
					stateobj.instptr=stateobj.calls.back();
					stateobj.calls.pop_back();
				});
		}
	}else if(*it==newline&&*++it==newline){
#ifdef DEBUG
		std::cout<<"[n][n][n]"<<std::endl;
#endif
		return wsobject(
			[](state& stateobj)
			{
				stateobj.instptr=stateobj.instructions.end();
			});
	}
}

wsobject inputoutput(std::string::const_iterator& it,char space,char tab,char newline)
{
	if(*++it==space){
		if(*++it==space){
#ifdef DEBUG
			std::cout<<"[t][n][s][s]"<<std::endl;
#endif
			return wsobject(
				[](state& stateobj)
				{
					std::cout<<(char)stateobj.data.back();
					stateobj.data.pop_back();
					++stateobj.instptr;
				});
		}else if(*it==tab){
#ifdef DEBUG
			std::cout<<"[t][n][s][t]"<<std::endl;
#endif
			return wsobject(
				[](state& stateobj)
				{
					std::cout<<stateobj.data.back();
					stateobj.data.pop_back();
					++stateobj.instptr;
				});
		}
	}else if(*it==tab){
		if(*++it==space){
#ifdef DEBUG
			std::cout<<"[t][n][t][s]"<<std::endl;
#endif
			return wsobject(
				[](state& stateobj)
				{
					stateobj.data.push_back(std::cin.get());
					for(;std::cin.get()!='\n';) continue;
					++stateobj.instptr;
				});
		}else if(*it==tab){
#ifdef DEBUG
			std::cout<<"[t][n][t][t]"<<std::endl;
#endif
			return wsobject(
				[](state& stateobj)
				{
					base_unit_type number=0;
					std::cin>>number;
					stateobj.data.push_back(number);
					++stateobj.instptr;
				});
		}
	}
}

std::vector<wsobject> parse(const std::string& program,char space=' ',char tab='\t',char newline='\n')
{
	std::vector<wsobject> instructions;
	for(auto it=program.cbegin();it!=program.cend();++it){
		if(*it==space) instructions.push_back(stackcontrol(it,space,tab,newline));
		else if(*it==tab){
			if(*++it==space) instructions.push_back(arithmetic(it,space,tab,newline));
			else if(*it==tab) instructions.push_back(heapaccess(it,space,tab,newline));
			else if(*it==newline) instructions.push_back(inputoutput(it,space,tab,newline));
		}else if(*it==newline) instructions.push_back(flowcontrol(it,space,tab,newline));
	}
	return instructions;
}

int main(int argc,char** argv)
{
	if(argc<2){
		std::cout<<"no input file."<<std::endl;
		std::cout<<"wsi [filename]"<<std::endl;
		return 0;
	}
	std::ifstream sourcefile(argv[1]);
	sourcefile.seekg(0,std::ios::end);
	std::vector<char> buffer(sourcefile.tellg(),0);
	sourcefile.seekg(0,std::ios::beg);
	sourcefile.read(buffer.data(),buffer.size());
	std::string program(buffer.data());
	std::regex_replace(program,std::regex("\r\n"),std::string("\n"),std::regex_constants::match_not_eol);
	auto instructions=parse(program,' ','\t','\n');
	state progstate={
		std::deque<base_unit_type>(),
		std::deque<std::vector<wsobject>::iterator>(),
		std::vector<base_unit_type>(),
		instructions
	};
	progstate.instptr=progstate.instructions.begin();
	std::cout<<"parse complete."<<std::endl;
	while(progstate.instptr!=progstate.instructions.end()){
		if(progstate.instptr->objecttype==wsobject::type::operation) progstate.instptr->function(progstate);
		else ++progstate.instptr;
	}
	return 0;
}
