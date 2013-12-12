clear

train_data=csvread('vowel_dataset_train.csv');
test_data=csvread('vowel_dataset_test.csv');
train=train_data(:,[end 1:end-1]);
test_labels=test_data(:,end);
test=test_data(:,1:end-1);

csvwrite('cross_validation_train.csv',train);
csvwrite('cross_validation_test.csv',test);

tree_counts=unique(round(logspace(0,5,10)));
for trees=1:length(tree_counts)    
    %disp('wrote out cross_validation_train.csv and cross_validation_test.csv')
tic
    [s w]=unix(['C++/forest_train cross_validation_train.csv ' num2str(tree_counts(trees)) ' cross_validation.forest']);
toc
tic
    [s w]=unix('C++/forest_eval cross_validation_test.csv cross_validation.forest cross_validation.classification');
toc

    prediction=csvread('cross_validation.classification');

    prediction_labels=prediction(1,:);
    prediction=prediction(2:end,:);

    [~, b]=max(prediction,[],2);
    prediction_labels=prediction_labels(b);

    accuracy(trees)=sum(prediction_labels'==test_labels)./length(test_labels);

    disp([num2str(tree_counts(trees)) ' trees, with the following accuracies:'])
    accuracy(trees)
    plot(accuracy)
    
    title(['accuracy for different number of trees'])
    set(gca,'XTick',1:trees)
    set(gca,'XTickLabel',tree_counts(1:trees))
    
    xlabel('# of trees in forest')
    ylabel('% accuracy')
    pause(1)
end