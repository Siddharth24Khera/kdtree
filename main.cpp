#include <iostream>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <sstream>
#include <queue>
#include <chrono>
#include <list>

using namespace std;

typedef vector<double> pnt;
typedef vector<vector<double>> points;
typedef vector<vector<vector<double>>> list_points;

struct intNode
{
	double data_median;
	double dist_to_query = std::numeric_limits<float>::infinity();
	bool is_leaf;
	pnt data_full_point; 		// level is depth%dimensions
	int level;
	intNode * l_intNode;
	intNode * r_intNode;
	points MBR;					//two points to define the rectangle(MBR)
	bool not_null = true;	
};

//dimensions start from 0
struct comparator {
	int dim1,totDim1;
    comparator(int dim,int totDim) { this->dim1 = dim; this->totDim1 = totDim;}
    bool operator () (std::vector<double> i, std::vector<double> j) { 
    	if (i[dim1]<j[dim1])
    		return true;
    	else if (i[dim1]>j[dim1])
    		return false;
    	else if (i[(dim1+1)%totDim1] < j[(dim1+1)%totDim1])
    		return true;
    	else if (i[(dim1+1)%totDim1] > j[(dim1+1)%totDim1])
    		return false;
    	else if (i[(dim1+2)%totDim1] < j[(dim1+2)%totDim1])
    		return true;
    	else if (i[(dim1+2)%totDim1] > j[(dim1+2)%totDim1])
    		return false;
    	else if (i[(dim1+3)%totDim1] <= j[(dim1+3)%totDim1])
    		return true;
    	else
    		return false;
    }
};

points sortPoints(points list, int dimension, int totalDim)
{	
	sort(list.begin(), list.end(), comparator(dimension,totalDim));
	return list;
}

void printPoints(points p)
{
	for(int i =0; i< p.size();i++){
		for (int j = 0; j < p[i].size(); j++)
		{
			cout<<p[i][j]<<' ';			
		}
		cout<<endl;
	}
}

bool compare(pnt i, pnt j, int dim1)
	{
		int totDim1 = i.size();
		if (i[dim1]<j[dim1])
			return true;
		else if (i[dim1]>j[dim1])
			return false;
		else if (i[(dim1+1)%totDim1] < j[(dim1+1)%totDim1])
			return true;
		else if (i[(dim1+1)%totDim1] > j[(dim1+1)%totDim1])
			return false;
		else if (i[(dim1+2)%totDim1] < j[(dim1+2)%totDim1])
			return true;
		else if (i[(dim1+2)%totDim1] > j[(dim1+2)%totDim1])
			return false;
		else if (i[(dim1+3)%totDim1] <= j[(dim1+3)%totDim1])
			return true;
		else
			return false;
	}


class kdTree
{
public:
	intNode* root;
	int dimension;
	kdTree(int dim)
	{
		dimension = dim;
	}


	void buildStart(points &all_data)
	{
		//sort on all dimensions, get d lists of points
		list_points sorted_lists;
		for(int i=0;i<dimension;i++)
			sorted_lists.push_back(sortPoints(all_data, i,dimension));
		intNode* head = new intNode;		
		points rect;
		for (int i = 0; i < 2; ++i)
		{
			pnt new_pnt;
			for (int j = 0; j < dimension; ++j)
			{
				if (i==0)				
					new_pnt.push_back(-1 * std::numeric_limits<float>::infinity());
				else	
					new_pnt.push_back(std::numeric_limits<float>::infinity());				
			}
			rect.push_back(new_pnt);
		}

		head->MBR = rect;
		buildTree(sorted_lists, head, 0, rect);
		root = head;
		sorted_lists.clear();
		sorted_lists.shrink_to_fit();
		return;
	}

	// Build the KD-tree
	void buildTree(list_points &sorted_lists, intNode* head, int level, points &rect)
	{
		//find median of levelth list -> access the size/2th element -> levelth 

		if (sorted_lists[level].size()==0)
		{
			head->not_null = false;
			head->is_leaf = false;
			return;
		}

		if (sorted_lists[level].size()==1)
		{
			head->data_full_point = sorted_lists[0][0];
			head->is_leaf=true;
			head->data_median = sorted_lists[0][0][level];
			head->l_intNode=NULL;
			head->r_intNode=NULL;
			head->level=level;
			points rect;
			rect.push_back(head->data_full_point);
			rect.push_back(head->data_full_point);
			head->MBR = rect;
			return;
		}
		
		int no_of_points = sorted_lists[level].size();
		double median = sorted_lists[level][floor((no_of_points-1)/2)][level];
		pnt median_point = sorted_lists[level][floor((no_of_points-1)/2)];

		head->data_median = median;
		head->is_leaf=false;
		head->MBR = rect;
		head->level = level;


		// partition all other lists on basis of earlier median into two new lists 
		list_points left_lists;
		list_points right_lists;

		for(int i=0;i<sorted_lists.size();i++)
		{
			points left_points;			
			points right_points;
			bool flag = false;
			for (int j=0;j<no_of_points;j++)
			{
				if(sorted_lists[i][j][level]<median)
				{
					left_points.push_back(sorted_lists[i][j]);
				}
				else if(sorted_lists[i][j][level]==median)
				{
					bool isLeft = compare(sorted_lists[i][j],median_point, level);
					if (isLeft)
						left_points.push_back(sorted_lists[i][j]);
					else right_points.push_back(sorted_lists[i][j]);

				}
				else right_points.push_back(sorted_lists[i][j]);
			}

			left_lists.push_back(left_points);
			right_lists.push_back(right_points);
		}


		intNode* leftNode = new intNode;
		intNode* rightNode = new intNode;
		
		points left_rect;
		points right_rect;
		left_rect = rect;
		left_rect[1][head->level] = head->data_median;
		right_rect = rect;
		right_rect[0][head->level] = head->data_median;		

		buildTree(left_lists, leftNode, (level+1)%dimension , left_rect);		//// HERE
		head->l_intNode=leftNode;

		left_lists.clear();
		left_lists.shrink_to_fit();

		buildTree(right_lists, rightNode, (level+1)%dimension, right_rect);		//// HERE
		head->r_intNode=rightNode;

		right_lists.clear();
		right_lists.shrink_to_fit();

		return;
	}
	~kdTree();
};

// // constructor and deconstructor have to be defined outside the class - o/w throws error in MacOS
// kdTree::kdTree()
// {
// 	//cout << "constructor" << endl;
// }

kdTree::~kdTree()
{
	//cout << "deconstructor" << endl;	
}

// min L2 distance of query point from MBR

double distance_from_mbr(pnt& data_point, points& MBR)
{
	int dim = data_point.size();
	pnt delta;
	double dist = 0;
	for (int i = 0; i < dim; ++i)
	{
		if (data_point[i] < MBR[0][i])
		{
			delta.push_back(MBR[0][i] - data_point[i]);
		}
		else if(data_point[i] > MBR[1][i])
		{
			delta.push_back(data_point[i] - MBR[1][i]);
		}
		else delta.push_back(0);
	}

	for (int i = 0; i < dim; ++i)
	{
		dist = dist + delta[i]*delta[i]	;
	}
	dist = sqrt(dist);
	return dist;
}


// L2 distance of query point from another point
double distance_from_point(pnt& query, pnt& data_point)
{
	int dim = data_point.size();
	double dist = 0;
	for (int i = 0; i < dim; ++i)
	{
		double delta = abs(query[i] - data_point[i]);
		dist = dist + delta*delta;
	}
	dist = sqrt(dist);
	return dist;
}


struct comparator_max_heap {
    bool operator () (pair<pnt, double> const& p1, pair<pnt, double> const& p2) 
    { 
    	if (p1.second < p2.second)
    		return true;
    	else if (p1.second > p2.second)
    		return false;
    	else return compare(p1.first,p2.first,0);
    }
};


struct comparator_min_heap {
    bool operator () (const intNode* n1, const intNode* n2) 
    { 
    	return n1->dist_to_query > n2->dist_to_query;
    }
};


bool changeNeeded(double dist_to_p2, pnt p2, priority_queue< pair<pnt,double>, vector<pair<pnt,double>>, comparator_max_heap> answer_set){
	if (answer_set.top().second > dist_to_p2)
	{
		return true;
	}
	else if(answer_set.top().second < dist_to_p2){
		return false;
	}		
	else{
		bool isLeft = compare(p2,answer_set.top().first,0);
		if (isLeft)
			return true;
		else return false;
	}
}

// KNN query - best first
priority_queue< pair<pnt,double>, vector<pair<pnt,double>>, comparator_max_heap> kNN_bestfirst(int k, pnt& query_point, intNode* head, points& all_points)
{
	// answer set:	initialise Max Heap of pnts of size k with k random points
	// candidate:	initilise Min Heap with root MBR
	// while MBR is closer to query than top node in answer_set OR candidate is not empty
		//	pop top MBR
		// if MBR is a leaf AND closer to query than top node in answer set
			// pop top of answer set and insert node in answer set
		// else
			// insert those children of MBR which are closer to query point than the top node in the answer set

	priority_queue< pair<pnt,double>, vector<pair<pnt,double>>, comparator_max_heap> answer_set;
	priority_queue<intNode*, vector<intNode*>, comparator_min_heap> candidate;
	// initialise wih first k points
	for (int i = 0; i < k; ++i)
	{
		answer_set.push(make_pair(all_points[i], distance_from_point(query_point, all_points[i])));
	}	
	head->dist_to_query = distance_from_mbr(query_point, head->MBR);

	candidate.push(head);
	while(!candidate.empty() && (candidate.top()->dist_to_query < answer_set.top().second) )
	{
		intNode* top_MBR = candidate.top();
		candidate.pop();
		if (top_MBR->is_leaf)
		{	
			pnt p2 = top_MBR->MBR[0];
			double max_dist = distance_from_point(query_point,p2);
			bool isChange = changeNeeded(max_dist,p2,answer_set);

			if(isChange){
				answer_set.pop();
				answer_set.push(make_pair(top_MBR->data_full_point, distance_from_point(query_point, top_MBR->data_full_point)));
			}
		}	
		else
		{
			intNode* leftChild = top_MBR->l_intNode;
			intNode* rightChild = top_MBR->r_intNode;
			double right_distance;
			if (rightChild->not_null == false){
				right_distance = std::numeric_limits<float>::infinity();
			}
			else
				right_distance = distance_from_mbr(query_point, rightChild->MBR);
		
			double left_distance = distance_from_mbr(query_point, leftChild->MBR);

			left_distance = distance_from_mbr(query_point, leftChild->MBR);			

			leftChild->dist_to_query = left_distance;
			rightChild->dist_to_query = right_distance;

			if (left_distance < answer_set.top().second)
			{
				candidate.push(leftChild);
			}
			if (right_distance < answer_set.top().second)
			{
				candidate.push(rightChild);
			}
		}
	}
	return answer_set;
}

priority_queue< pair<pnt,double>, vector<pair<pnt,double>>, comparator_max_heap> kNN_sequential_scan(int k, pnt& query_point, intNode* head, points& all_points)
{
	priority_queue< pair<pnt,double>, vector<pair<pnt,double>>, comparator_max_heap> answer_set;
	for (int i = 0; i < k; ++i)
	{
		answer_set.push(make_pair(all_points[i], distance_from_point(query_point, all_points[i])));
	}
	for (int i = k; i < all_points.size(); ++i)
	{	
		double dist = distance_from_point(query_point,all_points[i]);
		if(changeNeeded(dist,all_points[i],answer_set)){
			answer_set.pop();
			answer_set.push(make_pair(all_points[i], dist));
		}
	}

	return answer_set;

}

// Read data points from dataset.txt
points readData(string dataset_file)
{
	// ifstream dataFile("dataset.txt");
	ifstream dataFile(dataset_file);
	string line;
	int dimension = 0;
	int numberOfPoints = 0;
	points all_points;


	if (dataFile.is_open())
    {
    	int i = 0;

    	while (! dataFile.eof() )
    	{
    		stringstream ss;
      		getline (dataFile,line);
      		ss.str(line);

      		if(line == ""){
      			continue;
      		}

    		if (i==0){
    			ss>>dimension;
    			ss>>numberOfPoints;
    			i++;
    			continue;
    		}     		
    		vector<double> point;
    		for(int j=0;j<dimension;j++){
    			double x;
    			ss>>x;
    			point.push_back(x);
    		}
    		all_points.push_back(point);
     		i++;
    	}
    }

    return all_points;
}

list<pnt> write_result(priority_queue< pair<pnt,double>, vector<pair<pnt,double>>, comparator_max_heap> answer_set)
{	
	list<pnt> result;
	while(!answer_set.empty())
	{
		result.push_back(answer_set.top().first);
		answer_set.pop();
	}

	return result;
}

int main(int argc, char* argv[]) {

	char* dataset_file = argv[1];
	
	points all_points = readData(dataset_file);
	int dimension = all_points[0].size();
	bool isBest = (dimension < 10);

	kdTree mykdtree(dimension);
	mykdtree.buildStart(all_points);
	priority_queue< pair<pnt,double>, vector<pair<pnt,double>>, comparator_max_heap> answer_set;

	// Request name/path of query_file from parent by just sending "0" on stdout
	char* query_file = new char[100];
	int K;
	cout << 0 << endl;

	// Wait till the parent responds with name/path of query_file and k | Timer will start now
	cin >> query_file >> K;
	cerr << dataset_file << " " << query_file << " " << K << endl;

	// cin>>query_file>>K_neighbours;
	// int K = std::stoi(K_neighbours);
	// Read the query point from query_file, do kNN using the kdTree and output the answer to results.txt
	points all_query_points = readData(query_file);

	ofstream results_file;
	results_file.open("results.txt");

	for(int i = 0; i< all_query_points.size();i++)
	{

		if(isBest)
			answer_set =  kNN_bestfirst(K, all_query_points[i], mykdtree.root, all_points);
		else
			answer_set =  kNN_sequential_scan(K, all_query_points[i], mykdtree.root, all_points);		
		
		list<pnt> result = write_result(answer_set);
		int dim = answer_set.top().first.size();
		for (int i = 0; i < answer_set.size(); ++i)
		{
			vector<double> res = result.back();
			for (int j = 0; j < dim; ++j)
			{
				results_file<< res[j]<<" ";
			}
			result.pop_back();
			results_file<<"\n";

		}

	}

	results_file.close();
	// Convey to parent that results.txt is ready by sending "1" on stdout | Timer will stop now and this process will be killed
	cout << 1 << endl;
}
