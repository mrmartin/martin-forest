clear

%convert all instances of ? to NaN
[s w]=unix('sed -i s/?/NaN/g Wisconsin_Breast_Cancer.csv');

all_data=csvread('Wisconsin_Breast_Cancer.csv');

%convert all instances of NaN to ?
[s w]=unix('sed -i s/NaN/?/g Wisconsin_Breast_Cancer.csv');

folds=10;

shuffle=randperm(size(all_data,1));
steps=round(linspace(1,size(all_data,1),folds+1));
tree_counts=unique(round(logspace(0,5,50)));
for trees=1:length(tree_counts)
    for i=1:folds
        %disp(['train is [1:' num2str(steps(i)-1) ' ' num2str(steps(i+1)+1) ':end]'])
        train=all_data([shuffle(1:steps(i)-1) shuffle(steps(i+1)+1:end)],:);
        train=train(:,[11 2:10]);
        %disp(['test is [' num2str(steps(i)) ':' num2str(steps(i+1)) ']'])
        test=all_data(shuffle(steps(i):steps(i+1)),:);
        test_labels=test(:,11);
        test=test(:,2:10);
        
        csvwrite('cross_validation_train.csv',train);
        csvwrite('cross_validation_test.csv',test);
        
        [s w]=unix('sed -i s/NaN/?/g cross_validation_train.csv');
        [s w]=unix('sed -i s/NaN/?/g cross_validation_test.csv');
        
        %disp('wrote out cross_validation_train.csv and cross_validation_test.csv')

        [s w]=unix(['C++/forest_train cross_validation_train.csv ' num2str(tree_counts(trees)) ' cross_validation.forest']);
        
        [s w]=unix('C++/forest_eval cross_validation_test.csv cross_validation.forest cross_validation.classification');

        prediction=csvread('cross_validation.classification');

        prediction_labels=prediction(1,:);
        prediction=prediction(2:end,:);

        [~, b]=max(prediction,[],2);
        prediction_labels=prediction_labels(b);

        accuracy(trees,i)=sum(prediction_labels'==test_labels)./length(test_labels);
    end
    disp([num2str(tree_counts(trees)) ' trees, with the following accuracies:'])
    accuracy(trees,:)
    plot(mean(accuracy,2))
    hold on
    for i=1:folds
        plot(accuracy(:,i),'rx')
    end
    hold off
    
    title(['mean accuracy in ' num2str(folds) '-fold cross validation'])
    set(gca,'XTick',1:trees)
    set(gca,'XTickLabel',tree_counts(1:trees))
    
    xlabel('# of trees in forest')
    ylabel('% accuracy')
    pause(1)
end
%folds=10, 10 trees, mean(accuracy)=0.9613
%folds=10, 100 trees, mean(accuracy)=0.9570

figure
benchmark_forest_vowels