#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <regex>
#include <iterator>
#include <cassert>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h> 


//why on earth isn't this in the STL! (STL is the C++ standard library)
void string_trim(std::string &str, std::string trimchars=" \n\r\t"){
	const int first=str.find_first_not_of(trimchars); //first is the offset of the first occurance of a char not of the trimchars
	if(first==std::string::npos){ str=""; return;} //std::string::npos is when none of the trimchars are found
	const int last=str.find_last_not_of(trimchars); //similar to the above but by this point we know the return value isn't ""
	str.erase(last+1);
	str.erase(0,first);
}

//generic programming, in this case T is a string. Really this is a bit inefficient and the class should have two types, one to hold the previous characters, and one for the result. It's okay though ^_^
template <class T>
class MarkovChain{
    
    //struct and class are basically the same thing, this is a subclass
	struct InvMap{
		//basic idea: if there were N total outcomes then select a random
		//number from 0 to N-1 and take the outcome indexed by that number
		//counter is used to calculate which "region" the random number
		//falls in and get an index. Then result is the lookup table
		//once the index is found.
		uint32_t c;
		std::vector<uint32_t> counter;
		std::vector<T> result;
		
		InvMap()=default;
		
        //The constructor.
		InvMap(std::map<T, uint32_t> probs)
		:c(0) //this is part of the "standard initialiser list" and sets c to zero.
		{
            // i runs through each element of the map (which is essentially a dictionary). i.first is the index
            // i.second is the target (i.e. the number of occurances of of the Ts)
			for(auto &i:probs){
				c+=i.second; //c is therefore a running counter of all the occurances
				counter.push_back(c); //counter contains the cumulative total of the occurances
				result.push_back(i.first); //result contains what each occurance maps to
			}
		}
		
		T GetRand(){
			const uint32_t &r=rand() % c; //r is a random integer between 0 and <c
			const uint32_t &index=std::upper_bound(counter.begin(),counter.end(),r)-counter.begin();
            //std::upper_bount is an algorighm in the STL that assumes that counter is an ordered list (which it is, see the constructor) then returns the offset of the
            //least upper bound to c. This algorithm is implemented probably using some binary search technique so it'll be O(log n)
            //I don't know how other people implement markov chains but O(log n) is pretty good
			return result[index];
		}
	};
	
    //This will be the target of the dictionary for the markov chain. The "expensive" part of this algorithm is calculating the imap,
    //but this only needs to be done when there are new probabilities added to the chain so the calcualtion is cached for efficiency.
	struct Target{
		//flag to say if imap needs re-calculating
		bool inverted;
		std::map<T, uint32_t> map;
		InvMap imap;
	};
	
    //Finally, with the helper classes (structs) defined the probability dictionary can be defined :-D
    //This maps each index to a dictionary of probabilities along with a cached imap for actually running the chain.
	std::map<T,Target> probs;
	
    
	public:
	MarkovChain()=default; //default constructor for the chain
	
	void Update(const T& previous,const T& next){ //previous is what the previous characters look like, and next is what the next character looks like (assuming T is a string)
		const auto& it1 = probs.insert(std::make_pair(previous, Target())).first;
		//it1 now points to probs[previous], creating it if necessary
		
		//due to the update the imap is invalid, so flag it as so
		it1->second.inverted=false;
		
		//insert / increment the counter for the next event
		const auto& it2 = it1->second.map.insert(std::make_pair(next,0)).first;
        //it2 points to the location of the inserted element, but if the element already exists it points to the existing one and no insertion is done.
		it2->second++;
	}
	
	T Generate(const T& element){
		auto it=probs.find(element);
		assert(it!=probs.end()); //Probably not the best way to handle this case!! The assumption is this will never be called on anythign that's not in the chain.
		
		//if imap is invalid then generate a new one
		if(!it->second.inverted){
			it->second.imap=InvMap(it->second.map);
			it->second.inverted=true;
		}
		
		//return random element
		return it->second.imap.GetRand();
	}
	
	//prints stuff for debugging purposes.
	void Print(){
		for(auto &i:probs){
			std::cout << i.first << std::endl;
			for(auto &j:i.second.map)
				std::cout << "-- " << j.first << " (" << j.second << ")" << std::endl;
		}
	}
};


int main(){
	
	
	
	constexpr int order=3;

	srand (time(NULL)); //lazy C-style random :P

	MarkovChain<std::string> markovChain;
	std::ifstream infile("m_names.txt");
	std::string line;
	while (std::getline(infile, line))
	{
		int s=line.size();
		
		//easiest to discard chains less than the order
		if(s<order) continue;
		
        //the "^" and "$" are used as special chars to indicate the start and end of strings. This is okay for a name generator as these chars don't occur in people's names
        //if they did then that'd screw with this!
		for(int i=0;i<order;++i)
			markovChain.Update("^"+line.substr(0,i),line.substr(i,1));
		markovChain.Update(line.substr(s-order,order),"$");
		
        //now to fill the chain for the middle of the names
		for(int i=0;i<s-order;++i)
			markovChain.Update(line.substr(i,order),line.substr(i+order,1));
	}
		
	for(int i=0;i<100;i++){
		line="^";
		for(int j=1;j<order;++j)
			line+=markovChain.Generate(line);
		while(line.back()!='$')
			line+=markovChain.Generate(line.substr(line.size()-order,order));
		
		line=line.substr(1,line.size()-2);
		std::transform(++line.begin(), line.end(), ++line.begin(), ::tolower);
		std::cout << line << std::endl;
	}
	
	//markovChain.Print();


	return 0;
}


