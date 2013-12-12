#include <random>

std::vector<std::vector<int> > partition (int l){
	std::vector<std::vector<int> > partitions;
	int count=0;
	int start=rand()%(int)pow(3,l);
	printf("start at %d, max is %d\n",start,(int)pow(3,l));
	//it's possible that the start is so high that there is no acceptable solution beyond it.
	for (int i=start;i<pow(3,l);i++){
		int t=i;
		int c=l;
		int sel[l];
		for (int j=0;j<l;j++)
			sel[j]=0;

		while(t>0){
			c--;
			sel[c]=t%3;
			t=t/3;				
		}
		/*for (int j=0;j<l;j++)
			printf("%d",sel[j]);
		printf(" - i=%d",i);*/
		//condition 1: contains at least 2 non-zeros
		int s=0;
		//condition 2: contains both 1 and 2
		int ones=0;
		int twos=0;
		//condition 3: first non-zero digit is 1 (removes duplicates)
		int first=0;
		for (int j=0;j<l;j++){
			if(sel[j]!=0)
				s++;
			if(sel[j]==1)
				ones++;
			if(sel[j]==2)
				twos++;
			if(first==0 && sel[j]!=0)//first is not set yet, and the current is not 0
				first=sel[j];
		}
		if(s>=2 && ones>0 && twos>0){//don't care about condition 3 && first==1){
			count++;
			std::vector<std::vector<int> > partitions;
			partitions.push_back(std::vector<int>());
			partitions.push_back(std::vector<int>());

			/*for (int j=0;j<l;j++)
				printf("%d",sel[j]);*/

			for (int j=0;j<l;j++)
				if(sel[j]==1)
					partitions[0].push_back(j);
				else if(sel[j]==2)
					partitions[1].push_back(j);

			/*for(std::vector<int>::size_type i = 0; i != partitions[0].size(); i++) {
				printf("%d",partitions[0][i]);
			}
			printf(" - ");
			for(std::vector<int>::size_type i = 0; i != partitions[1].size(); i++) {
				printf("%d",partitions[1][i]);
			}

			printf("\n");*/
			return partitions;
		}
	}
	//if no acceptable solution was found until here, start again from 0
	for (int i=0;i<pow(3,l);i++){
		int t=i;
		int c=l;
		int sel[l];
		for (int j=0;j<l;j++)
			sel[j]=0;

		while(t>0){
			c--;
			sel[c]=t%3;
			t=t/3;				
		}
		/*for (int j=0;j<l;j++)
			printf("%d",sel[j]);
		printf(" - i=%d",i);*/
		//condition 1: contains at least 2 non-zeros
		int s=0;
		//condition 2: contains both 1 and 2
		int ones=0;
		int twos=0;
		//condition 3: first non-zero digit is 1 (removes duplicates)
		int first=0;
		for (int j=0;j<l;j++){
			if(sel[j]!=0)
				s++;
			if(sel[j]==1)
				ones++;
			if(sel[j]==2)
				twos++;
			if(first==0 && sel[j]!=0)//first is not set yet, and the current is not 0
				first=sel[j];
		}
		if(s>=2 && ones>0 && twos>0){//don't care about condition 3 && first==1){
			count++;
			std::vector<std::vector<int> > partitions;
			partitions.push_back(std::vector<int>());
			partitions.push_back(std::vector<int>());

			/*for (int j=0;j<l;j++)
				printf("%d",sel[j]);*/

			for (int j=0;j<l;j++)
				if(sel[j]==1)
					partitions[0].push_back(j);
				else if(sel[j]==2)
					partitions[1].push_back(j);

			/*for(std::vector<int>::size_type i = 0; i != partitions[0].size(); i++) {
				printf("%d",partitions[0][i]);
			}
			printf(" - ");
			for(std::vector<int>::size_type i = 0; i != partitions[1].size(); i++) {
				printf("%d",partitions[1][i]);
			}

			printf("\n");*/
			return partitions;
		}
	}
	return partitions;
}

int main( int argc, char** argv )
{
	int l;
	if( argc != 2)
	{
		printf( "\trequires number of elements argument\n" );
		return -1;
	}else{
		l=atoi(argv[1]);
	}

	printf( "\trequires number of elements argument\n" );

	srand(time(NULL)); /* seed random number generator */
	printf("%d, %d\n", RAND_MAX, rand());

	std::vector<std::vector<int> > partitions = partition(l);
	for(std::vector<int>::size_type i = 0; i != partitions[0].size(); i++) {
		printf("%d ",partitions[0][i]);
	}
	printf(" - ");
	for(std::vector<int>::size_type i = 0; i != partitions[1].size(); i++) {
		printf("%d ",partitions[1][i]);
	}

	printf("\n");

	/*printf("Finding permutations for %d elements:\n",l);

	int count=0;
	for (int i=0;i<pow(3,l);i++){
		int t=i;
		int c=l;
		int sel[l];
		for (int j=0;j<l;j++)
			sel[j]=0;

		while(t>0){
			c--;
			sel[c]=t%3;
			t=t/3;				
		}
		//for (int j=0;j<l;j++)
		//	printf("%d",sel[j]);
		//printf(" - i=%d",i);

		//condition 1: contains at least 2 non-zeros
		int s=0;
		//condition 2: contains both 1 and 2
		int ones=0;
		int twos=0;
		//condition 3: first non-zero digit is 1 (removes duplicates)
		int first=0;
		for (int j=0;j<l;j++){
			if(sel[j]!=0)
				s++;
			if(sel[j]==1)
				ones++;
			if(sel[j]==2)
				twos++;
			if(first==0 && sel[j]!=0)//first is not set yet, and the current is not 0
				first=sel[j];
		}
		if(s>=2 && ones>0 && twos>0 && first==1){
			count++;
			std::vector<std::vector<int> > partitions;
			partitions.push_back(std::vector<int>());
			partitions.push_back(std::vector<int>());

			//for (int j=0;j<l;j++)
			//	printf("%d",sel[j]);

			for (int j=0;j<l;j++)
				if(sel[j]==1)
					partitions[0].push_back(j);
				else if(sel[j]==2)
					partitions[1].push_back(j);

			for(std::vector<int>::size_type i = 0; i != partitions[0].size(); i++) {
				printf("%d",partitions[0][i]);
			}
			printf(" - ");
			for(std::vector<int>::size_type i = 0; i != partitions[1].size(); i++) {
				printf("%d",partitions[1][i]);
			}

			printf("\n");
		}
	}
	printf("Turns out there are %d combinations\n",count);*/

	return 0;
}
