all: forest_eval forest_train

CXXFLAGS=-std=gnu++11 `pkg-config eigen3 --cflags`
LIBS=`pkg-config eigen3 --libs`

forest_eval: evaluate_multiclass_forest.o
	g++ -o $@ $^ $(CXXFLAGS) $(LIBS)

forest_train: train_multiclass_forest.o
	g++ -o $@ $^ $(CXXFLAGS) $(LIBS)

install: all
	install forest_eval /usr/local/bin/forest_eval
	install forest_train /usr/local/bin/forest_train

test: all
	./forest_train ../Wisconsin_Breast_Cancer_train.csv 5 breast.forest
	./forest_eval ../Wisconsin_Breast_Cancer_test_nolabels.csv breast.forest breast.classification


clean:
	rm -f *.o forest_eval forest_train