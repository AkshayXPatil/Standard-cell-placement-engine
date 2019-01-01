#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include<cmath>
using namespace std;
int new_cost;
int row;
int col;
int tot;
int cost=0;
int limit=10000;
string x1_y1,x2_y2;
int cnt=0;
int curr_cell=0;
int Wx,Wy;
 
map <string, bool> duplicate_cell_location;		//holds a true value for a key of unique location id, if the location is occupied by a cell
ofstream logfile("Team09_results.txt");
ofstream results("result.csv");

vector<string> split(string strToSplit, char delimeter)
{
    stringstream ss(strToSplit);
    string item;
    vector<string> splittedStrings;
    while (getline(ss, item, delimeter))
    {
       splittedStrings.push_back(item);
    }
    return splittedStrings;
}


int toint(string strToint)
{
	stringstream ss(strToint);
	int item = 0;
	ss>>item;
	return item;
}

string toStr ( int X, int Y )
{
	ostringstream ss;
	ss << X<<","<<Y;
	return ss.str();
}

/*function to tune the aspect ratio close to 1*/
int selectAR(int &row, int &col, int &tot, int &blanks)
{
	int row1,row2,col1,col2, blanks1, blanks2;
	float AR;
	row1 = row-1;
	row2 = row+1;
	/*tuning rows*/
	do
	{
		col1 = ceil((float)tot/row1);
		AR = ((8*round(row1))+8)/(float)col1;
		blanks1 =(row1*col1)-tot;
		if((AR>=0.96) && (AR<=1.04))
		{
			if(blanks1 < blanks)
			{
				row = row1;
				col = col1;
				blanks = blanks1;
			}
		}
		row1=row1-1;
	}
	while(AR>=0.96);
		/*tuning columns*/
	do
	{
		col2 = ceil((float)tot/row2);
		AR = ((8*round(row2))+8)/(float)col2;
		blanks2 = (row2*col2)-tot;
		if((AR>=0.96) && (AR<=1.04))
		{
			
			if(blanks2 < blanks)
			{
				row = row2;
				col = col2;
				blanks = blanks2;
			}
		}
		row2=row2+1;
	}
	while(AR<= 1.04);
}



class net 
{
	public:
	
	//May change after the move
	map <int,int>x_coord;
	map <int,int>y_coord;
	int length;
	
	//Will not change even after the move
	map <int,bool>cell_list;	// key: cell_no, value: true if that cell present on that cell
	int cell_count;
	
	//CONSTRUCTOR
	net()
	{
		cell_count = 0;
		cell_list.clear();
		x_coord.clear();
		y_coord.clear();
		length = 0;
	}
};

class node
{
	public:
	
	//Will not change even after the move
	vector <int> net_list;
	
	//Will change after the move
	int x;
	int y;
};



map <int, net> net_map; 			//primary data structure for all nets 
map <int, net>::iterator net_it; 
map <int, node> cell;				//primary data structure for all cells
map <int, node>::iterator cell_it; 
map <string, int> from_location;	//map of (location id, cell no.) pair

/*function for net length calculation*/
int netlength(int &net_no)
{
	int xmax,xmin;
	int ymax,ymin;	
	map <int,int>::iterator it1;
	map <int,int>::iterator it2;
	int length=0;
	
	it1= net_map[net_no].x_coord.begin();
	xmin=it1->first;
	it1=net_map[net_no].x_coord.end();
	it1--;
	xmax=it1->first;
	
	it2= net_map[net_no].y_coord.begin();
	ymin=it2->first;
	it2= net_map[net_no].y_coord.end();
	it2--;
	ymax= it2->first;
	
	length = ((xmax-xmin)*4)+((ymax-ymin)*16);
	return(length);
}

/*function to make or undo the move*/
int check_lengths(bool swap, bool undo)
{
	int curr_net;
	int old_length;
	int new_length;
	int old_cost = cost;
	
	int delta_c;
	int i;
	float temp;
	int xmin,xmax;
	int ymin,ymax;
	map <int,int>::iterator it1;
	map <int,int>::iterator it2;
	int x2;
	int y2;
	bool blank1=false;
	bool blank2=false;
	int x1;
	int y1;
	
	new_cost = cost;
	
	if ( duplicate_cell_location[x1_y1] )	//checking if cell1 is not a blank location
	{
		x1=cell[from_location[x1_y1]].x;
		y1=cell[from_location[x1_y1]].y;
	}
	else
	{				
		vector<string> loc;
		loc = split(x1_y1,',');
		x1 = toint(loc[0]);
		y1 = toint(loc[1]);
		blank1 = true;
	}
	
	if ( duplicate_cell_location[x2_y2] ) 	//check if cell2 is not a blank location
	{
		x2=cell[from_location[x2_y2]].x;
		y2=cell[from_location[x2_y2]].y;
	}
	else
	{
		vector<string> loc;
		loc = split(x2_y2,',');
		x2 = toint(loc[0]);
		y2 = toint(loc[1]);
		blank2 = true;
	}

	if(!blank1)		//if cell1 is not blank, calculate lengths of the all the nets connected to cell1
	{
		for(i=0;i<cell[from_location[x1_y1]].net_list.size();i++)
		{
			curr_net=cell[from_location[x1_y1]].net_list[i];
			if(!blank2)			//cell2 is not blabnk
			{
				if(net_map[curr_net].cell_list[from_location[x2_y2]]) //cell2 is on the same net
				{
					continue;			// no need to calculate net length
				}
			}
			
			if(!undo)				
			{
				old_length = net_map[curr_net].length;
				it1 = net_map[curr_net].x_coord.begin();
				it2 = net_map[curr_net].x_coord.end(); 		//points to the past the end of the map
				it2--; 										// points to the actual end of the net
				xmin = it1->first; 							//current min
				xmax = it2->first; 							//current max
				if(x2<xmin) 								//is future min less than current min?
				{
					xmin = x2;
				}
				else if(x1 == xmin )						//is current cell one of the horizontal extremes? 
				{
					if(net_map[curr_net].x_coord[x1] == 1) 	//is that the only cell?
					{
						it1++;
						if(x2<=it1->first)					//comparing potential xmin
						{
							xmin=x2;
						}
						else
						{	
							xmin = it1->first;
						}
					}
				}
				
				if(x2>xmax)									//is future max greater than the current max?
				{
					xmax=x2;
				}
				else if(x1 == xmax)							//is current cell one of the horizontal extremes?
				{
					
					if(net_map[curr_net].x_coord[x1] == 1)	//is that the only cell?
					{
						it2--;
						if(x2>=it2->first)  				//comparing potential xmax
						{
							xmax=x2;
						}
						else
						{
							xmax=it2->first;
						}
					}	
				}
				
				
				it1 = net_map[curr_net].y_coord.begin();
				it2 = net_map[curr_net].y_coord.end();
				it2--;
				ymin = it1->first; 							//current min
				ymax = it2->first;                          //current max
				if(y2<ymin)		                            //is future min less than current min?
				{                                           
					ymin = y2;                              
				}                                           
				else if(y1 == ymin)                         //is current cell one of the vertical extremes?
				{                                           
					if(net_map[curr_net].y_coord[y1] == 1)  //is that the only cell?
					{                                       
						it1++;                              
						if(y2<=it1->first)                  //comparing potential ymin
						{                                   
							ymin=y2;                        
						}                                   
						else                                
						{	                                
							ymin = it1->first;              
						}                                   
					}                                       
				}                                           
				                                            
				if(y2>ymax)                                 //is future max greater than the current max?
				{                                           
					ymax=y2;                                
				}	                                        
				if(y1 == ymax )                             //is current cell one of the vertical extremes?
				{                                           
					if(net_map[curr_net].y_coord[y1] == 1)  
					{                                       //is that the only cell?
						it2--;                              
						if(y2>=it2->first)                  
						{                                   //comparing potential ymax
							ymax=y2;
						}
						else
						{
							ymax=it2->first;
						}
					}
				}
				new_length=((xmax-xmin)*4)+((ymax-ymin)*16);
				new_cost=new_cost-old_length+new_length;
			}

			if ( swap )
			{
				if(!blank2)				
				{
					if( net_map[curr_net].cell_list[from_location[x2_y2]] )		//cell2 is blank and is on the same net
					{
						continue;    //no need to calculate length
					}
				}
				
				net_map[curr_net].x_coord[x1]--;			//update x coordinates for the net
				if(net_map[curr_net].x_coord[x1] == 0)
				{
					net_map[curr_net].x_coord.erase(x1);
				}
				net_map[curr_net].x_coord[x2]++;
				
				net_map[curr_net].y_coord[y1]--;			//update y coordinates for the net
				if(net_map[curr_net].y_coord[y1] == 0)
				{
					net_map[curr_net].y_coord.erase(y1);
				}
				net_map[curr_net].y_coord[y2]++;
				
				new_length=netlength(curr_net);				//calculate netlength
				old_length = net_map[curr_net].length;
				new_cost=new_cost-old_length+new_length;
			
				net_map[curr_net].length = new_length;
				cost = new_cost;
			}
		}
	}
	
	if ( !blank2 )
	{
		for(i=0;i<cell[from_location[x2_y2]].net_list.size();i++)
		{
			curr_net=cell[from_location[x2_y2]].net_list[i];
			if(!blank1)
			{
				if(net_map[curr_net].cell_list[from_location[x1_y1]])
				{
					continue;
				}
			}
			if(!undo)
			{
				old_length = net_map[curr_net].length;
				it1 = net_map[curr_net].x_coord.begin();			
				it2 = net_map[curr_net].x_coord.end();    					//points to the past the end of the map          
				it2--;                                                      // points to the actual end of the net
				xmin = it1->first;                                          //current min
				xmax = it2->first;                                          //current max
				if(x1<xmin)                                                 //is future min less than current min?
				{                                                           
					xmin = x1;                                              
				}                                                           
				else if(x2 == xmin )                                        //is current cell one of the horizontal extremes?
				{                                                           
					if(net_map[curr_net].x_coord[x2] == 1)                  //is that the only cell?
					{                                                       
						it1++;                                              
						if(x1<=it1->first)                                  //comparing potential xmin
						{                                                   
							xmin=x1;                                        
						}                                                   
						else                                                
						{	                                                
							xmin = it1->first;                              
						}                                                   
					}                                                       
				}                                                           
				
				if(x1>xmax)                                                 //is future max greater than the current max?
				{                                                           
					xmax=x1;                                                
				}                                                           
				else if(x2 == xmax)                                         //is current cell one of the horizontal extremes?
				{                                                           
				
					if(net_map[curr_net].x_coord[x2] == 1)                  //is that the only cell?
					{                                                       
						it2--;                                              
						if(x1>=it2->first)                                  //comparing potential xmax
						{                                                   
							xmax=x1;                                        
						}                                                   
						else                                                
						{                                                   
							xmax=it2->first;                                
						}                                                   
					}	                                                    
				}
				
				it1 = net_map[curr_net].y_coord.begin();                    
				it2 = net_map[curr_net].y_coord.end();                      
				it2--;                                                      
				ymin = it1->first;                                          //current min
				ymax = it2->first;                                          //current max
				if(y1<ymin)                                                 //is future min less than current min?
				{                                                           
					ymin = y1;                                              
				}                                                           
				else if(y2 == ymin)                                         //is current cell one of the vertical extremes?
				{                                                           
					if(net_map[curr_net].y_coord[y2] == 1)                  //is that the only cell?
					{                                                       
						it1++;                                              
						if(y1<=it1->first)                                  //comparing potential ymin
						{                                                   
							ymin=y1;                                        
						}                                                   
						else                                                
						{	                                                
							ymin = it1->first;                              
						}                                                   
					}                                                       
				}                                    
                
				if(y1>ymax)                                                 //is future max greater than the current max?
				{                                                           
					ymax=y1;                                                
				}	                                                        
				if(y2 == ymax )                                             //is current cell one of the vertical extremes?
				{                                                   
					if(net_map[curr_net].y_coord[y2] == 1)          //is that the only cell?
					{                                               
						it2--;                                      
						if(y1>=it2->first)                          //comparing potential ymax
						{
							ymax=y1;
						}
						else
						{
							ymax=it2->first;
						}
					}
				}
				new_length=((xmax-xmin)*4)+((ymax-ymin)*16);
				new_cost=new_cost-old_length+new_length;
			}

			if(swap)
			{
				if(!blank1)
				{
					if( net_map[curr_net].cell_list[from_location[x1_y1]] )
					{
						continue;
					}
				}
				
				net_map[curr_net].x_coord[x2]--;				//update x coordinates for the net
				if(net_map[curr_net].x_coord[x2] == 0)          
				{                                               
					net_map[curr_net].x_coord.erase(x2);        
				}                                               
				net_map[curr_net].x_coord[x1]++;        
				
				net_map[curr_net].y_coord[y2]--;                //update y coordinates for the net
				if(net_map[curr_net].y_coord[y2] == 0)          
				{                                               
					net_map[curr_net].y_coord.erase(y2);        
				}                                               
				net_map[curr_net].y_coord[y1]++;           
				
				new_length = netlength(curr_net);               //calculate netlength
				old_length = net_map[curr_net].length;
				new_cost=new_cost-old_length+new_length;
			
				net_map[curr_net].length = new_length;
				cost = new_cost;
			}
		}
	}

	if (swap)
	{
		int temp;
		if(!blank1 && !blank2)		//swap cell1 and cell2
		{
			temp = cell[from_location[x1_y1]].x;
			cell[from_location[x1_y1]].x = cell[from_location[x2_y2]].x;
			cell[from_location[x2_y2]].x = temp;
			
			temp = cell[from_location[x1_y1]].y;
			cell[from_location[x1_y1]].y = cell[from_location[x2_y2]].y;
			cell[from_location[x2_y2]].y = temp;
			
			temp = from_location[x1_y1];
			from_location[x1_y1] = from_location[x2_y2];
			from_location[x2_y2] = temp;
		}
		else if(blank1)				//swap cell2 and blank
		{
			cell[from_location[x2_y2]].x = x1;
			cell[from_location[x2_y2]].y = y1;
			from_location[x1_y1] = from_location[x2_y2];
			from_location.erase(x2_y2);
			duplicate_cell_location[x1_y1] = true;
			duplicate_cell_location[x2_y2] = false;
		}
		else if(blank2)				//swap cell1 and blank
		{
			cell[from_location[x1_y1]].x = x2;
			cell[from_location[x1_y1]].y = y2;
			from_location[x2_y2] = from_location[x1_y1];
			from_location.erase(x1_y1);
			duplicate_cell_location[x2_y2] = true;
			duplicate_cell_location[x1_y1] = false;
		}
		
	}
	
	delta_c = new_cost-old_cost;	//delta_c calculation

	return delta_c;
}

/*function bellow returns true if cells to be swapped are in range*/
bool inwindow(int x1, int x2, int y1, int y2)
{
	if((abs(x2-x1)<=Wx) && (abs(y2-y1)<=Wy))
	{
		return true;
	}
	else
	{
		return false;
	}	
}

/*generates random two locations to be swapped*/
int generate()
{
	int x1,x2;
	int y1,y2;
	
	do
	{
		x1 = rand()%col;
		y1 = 2*(rand()%row)+1;
		x1_y1 = toStr(x1,y1);

	}while(!duplicate_cell_location[x1_y1]);    //cell1 has to be a cell
	
	do
	{
		x2 = rand()%col;
		y2 = 2*(rand()%row)+1;
		x2_y2 = toStr(x2,y2);
		
	}while((x1_y1 == x2_y2) && (!inwindow(x1,x2,y1,y2)) ); //cell2 is not cell1 and is in window

}

/*function to decide if the move is acceptable at current temperature*/
bool accept(int &delta_c, float &T)
{
	float prob;
	float r;
	int iprob;
	prob = exp((-delta_c)/(float)T);

	r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);	//generates a random no. between 0-1
	
	float one = 1.0;
	if( r < min(prob,one) )
	{
		results<<cost<<endl;
		return true;	
	}
	else
	{	
		results<<","<<cost<<endl;
		return false;
	}
}
int main (int argc, char* argv[]) 
{
	string line;
	vector<string> netline;
	vector<string> nodeline;
	map <int,bool> duplicate_cell;		//holds a true value for every cell no. present in the circuit
	int cell_no=0;
	int net_no=0;
	float AR;
	int blanks=0;
	int x = 0;
	int y = 1;
	string x_y;
	int delta_c=0;
	float T,T1;
	int innerLimit;
	bool undo = true;
	bool swap = true;
	bool flag = false;
	
	srand(time(NULL));
	time_t start_time;
	time_t time1;
	
	if(argc < 2)
	{
	  cout<<"please specify the input file name\n";
	  return 0;
	}
	
	const char* arg = argv[1];
	
	ifstream inputfile(arg);
	if(inputfile.is_open())
	{
		for(int i =0; i<5; i++)
		{
			getline (inputfile,line); //line#5 gives total no. of cells in the circuit 
		}
		tot = toint(line)+1;
		row = round((-4 + sqrt(16 + 32 * tot)) / 16);
		
		if ( tot <500 )		//if circuit consists of less than 500 cells
		{
			innerLimit = 3000;
		}
		else				//circuit consists of more than 500 cells
		{
			innerLimit = tot*2.5;
		}
		
		col = ceil((float)tot/row);
		AR = ((8*round(row))+8)/(float)ceil(col);
		blanks =(row*col)-tot;
		
		selectAR(row,col,tot,blanks);		//Tune aspect ratio
		
		AR = ((8*round(row))+8)/(float)ceil(col);	
		blanks =(row*col)-tot;
		Wx = col*4;
		Wy = (2*row+1)*16;
		
		
		logfile<<"total cells: "<<tot<<endl;
		logfile<<"rows: "<<row<<endl;
		logfile<<"columns: "<<col<<endl;
		logfile<<"AR: "<<AR<<endl;
		logfile<<"blanks: "<<blanks<<endl;
	
		while(getline (inputfile,line))
		{
			if(line.find("s")!= string::npos)
			{
				if(net_map[net_no].cell_count>1)		
				{
					net_map[net_no].length = netlength(net_no); //calculating net length 
					cost = cost+net_map[net_no].length ;
					net_no++;
				}
				if(net_map[net_no].cell_count==1)				//ignoring nets with only 1 cell
				{
					cell[cell_no].net_list.pop_back();
					net_map[net_no].cell_list[cell_no] = false;
					net_map[net_no].cell_count--;
					if(net_map[net_no].cell_count == 0)		
					{
						net_map.erase(net_no);
					}
				}
			}
	
			if(line.find("a") != string::npos)
			{
				net_map[net_no].cell_count++;
				nodeline = split(line, ' ');
				nodeline = split(nodeline[0], 'a');
				cell_no = toint(nodeline[1]);
				
				cell[cell_no].net_list.push_back(net_no); 		//adding the net to the netlist of the cell
				
				net_map[net_no].cell_list[cell_no] = true;
				
				if(!duplicate_cell[cell_no]) 			 		//if cell is new
				{
					duplicate_cell[cell_no] = true;
					do
					{
						x = rand()%col;					  //pick random x location
						y = 2*(rand()%row)+1;			  //pick random y location
						x_y = toStr(x,y);				  //create a unique location id
					}while(duplicate_cell_location[x_y]); //initial random placement
					
					cell[cell_no].x=x;
					cell[cell_no].y=y;
					from_location[x_y]=cell_no;
					duplicate_cell_location[x_y] = true;
				}
				
				net_map[net_no].x_coord[cell[cell_no].x]++;
				net_map[net_no].y_coord[cell[cell_no].y]++;
				
			} 
		}
		inputfile.close();
		
		if(net_map[net_no].cell_count==1)				//ignoring if last net has only one cell
		{
			cell[cell_no].net_list.pop_back();
			net_map[net_no].cell_count--;
			if(net_map[net_no].cell_count == 0)
			{
				net_map.erase(net_no);
			}	
		}
		else											//updating parameters for last net
		{
			net_map[net_no].length = netlength(net_no);		
			cost = cost+net_map[net_no].length ;
		}
		
	}
	
	logfile<<"initial cost: "<<cost<<endl;
	
	T = 40000;		//Initial temperature
 

	/*
	ofstream tem ("temperature.csv");
	time (&start_time);
	tem<<"Iteration#, Temperature, InternalLimit, End cost, Time\n";
	tem<<cnt<<","<<T<<","<<innerLimit<<","<<cost<<", "<<ctime(&start_time); 
	*/

	while(T>0.1)
	{
		int count =0;	    //initializing internal iteration count for this temp.
		int rejected = 0;	//rejection count for this temp.
		int accepted = 0;	//acceptance count for this temp.
		
		while( count < innerLimit )
		{
			cnt++;			//total iteration count
			
			generate();		//generate two locations to swap
			
			if(!flag)		//flag is low since rejections < acceptance 
			{
				delta_c = check_lengths(swap,undo);			//do the swap and ignore calculation part
				if(!accept(delta_c,T))
				{
					check_lengths(swap,undo);				//rejected! swap again and undo
					rejected++;
				}
				else
				{
					accepted++;
				}
			}
			else			//flag is high since rejections > acceptance
			{
				delta_c = check_lengths(!swap,!undo);		//don't swap, check first
				if(accept(delta_c,T))
				{
					check_lengths(swap,undo);				//accepted!, do the swap
					accepted++;
				}
				else
				{
					rejected++;
				}
			}
			
			count++;	
		}
		
		/* 
		time (&time1);
		tem<<cnt<<","<<T<<","<<innerLimit<<","<<cost<<","<<difftime(time1,start_time)<<endl;
		*/
		
		if(accepted == 0)		//exit if nothing gets accepted
		{
			goto end;
		}
		
		else if(accepted < rejected)		
		{
			flag = true;
			if(T>4)
			{
				T1 = 0.95*(float)T;		//lower the rate of temp. reduction
			}
			else
			{
				T1 = 0.98*(float)T;		//temp. reducing at the lowest rate
			}
		}
	
		else
		{
			T1 = 0.80*(float)T;			//temp. reducing by 20%
		}
		
		Wx = Wx*(log(T1)/log(T));		//update window width
		Wy = Wy*(log(T1)/log(T));		//update window hight
		T= T1;
	}
	
end:logfile<<"final cost: "<<cost<<endl;
	logfile.close();
	return 0;
}