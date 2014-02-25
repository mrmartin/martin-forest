all: forest_eval forest_train

CXXFLAGS=-std=gnu++11

forest_eval: C++/evaluate_multiclass_forest.o
	g++ -o $@ $^ $(CXXFLAGS)

forest_train: C++/train_multiclass_forest.o
	g++ -o $@ $^ $(CXXFLAGS)

install: all
	install forest_eval /usr/local/bin/forest_eval
	install forest_train /usr/local/bin/forest_train

test: all
	./forest_train Wisconsin_Breast_Cancer_train.csv 5 breast.forest
	./forest_eval Wisconsin_Breast_Cancer_test_nolabels.csv breast.forest breast.classification usefulness.entropy

clean:
	rm -f *.o forest_eval forest_train
