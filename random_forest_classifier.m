%a random forest is a combination of random trees
%a random tree is a combination of random decision functions
load iris.dat
setosa = iris((iris(:,5)==1),:);        % data for setosa
versicolor = iris((iris(:,5)==2),:);    % data for versicolor
virginica = iris((iris(:,5)==3),:);     % data for virginica
obsv_n = size(iris, 1);                 % total number of observations
X=iris(:,1:4);
X=[ones(size(X,1),1) X];
Y=[repmat(1,50,1);repmat(2,50,1);repmat(3,50,1)];%categorical labels

%% plot data %%
Characteristics = {'sepal length','sepal width','petal length','petal width'};
pairs = [1 2; 1 3; 1 4; 2 3; 2 4; 3 4];
h = figure;
for j = 1:6,
    x = pairs(j, 1);
    y = pairs(j, 2);
    subplot(2,3,j);
    plot([setosa(:,x) versicolor(:,x) virginica(:,x)],...
         [setosa(:,y) versicolor(:,y) virginica(:,y)], '.');
    xlabel(Characteristics{x},'FontSize',10);
    ylabel(Characteristics{y},'FontSize',10);
end

%% given Y, calculate the entropy %%
H=0;
for i=unique(Y)'
    p=sum(Y==i)/length(Y);
    if(p>0)
        H=H+p*log(p);
    end
end
H

%% given Y and a linear classifier over X, calculate the entropy of the classified data %%
load iris.dat
X=iris;
Y=[repmat(1,50,1);repmat(2,50,1);repmat(3,50,1)];%categorical labels

w=rand(5,1);
Yp=X*w;
left=find(Yp>=mean(Yp));
right=find(Yp<mean(Yp));

lH=0;
for i=unique(Y(left))'
    p=sum(Y(left)==i)/length(Y(left));
    if(p>0)
        lH=lH+p*log(p);
    end
end

rH=0;
for i=unique(Y(right))'
    p=sum(Y(right)==i)/length(Y(right));
    if(p>0)
        rH=rH+p*log(p);
    end
end
rH+lH

%% train a binary linear classifier %%
un=unique(Y);
%divide n classes into 2 subsets in all possible ways
p=SetPartition(length(un),2);
H=zeros(size(p));
for i=1:size(p,1)
    %train a linear classifier for each, and calculate the resulting entropy
    t=zeros(size(Y));
    t(ismember(Y,un(p{i}{1})))=1;
    %t(Y==p{i}{2})=0;
    %t'
    w=t'/X'
    H(i)=classifier_entropy(w',X,Y)
end

%chose classifier which most decreased entropy
[~,i]=min(H);
t=zeros(size(Y));
t(ismember(Y,un(p{i}{1})))=1;
%t(Y==p{i}{2})=0;
w=(t'/X')';

%% train the whole decision tree %%
root=struct;
root.w=w;

root=train_decision_tree(X,Y);

%% make prediction with a decision tree %%
classify_random_tree(X,root)

%% train on random subset of data, and test on rest %%
for i=1:1000
    s=randperm(length(Y));
    trainX=X(s(1:75),:);
    trainY=Y(s(1:75),:);

    testX=X(s(75:end),:);
    testY=Y(s(75:end),:);

    tree=train_decision_tree(trainX,trainY);

    t=classify_random_tree(testX,tree);

    accuracy(i)=1-sum(t'-testY'~=0)/length(t);
end
subplot(1,2,1)
hist(accuracy)
title('accuracy distribution over 1000 trees with different data')

%% train entire forest, and use it to classify %%
for i=1:1000
    s=randperm(length(Y));
    trainX=X(s(1:75),:);
    trainY=Y(s(1:75),:);

    testX=X(s(75:end),:);
    testY=Y(s(75:end),:);

    forest=cell(100,1);
    for k=1:100
        forest{k}=train_decision_tree(trainX,trainY);
    end

    t=zeros(length(testY),100);
    for k=1:100
        t(:,k)=classify_random_tree(testX,forest{k});
    end

    t=median(t,2);
    accuracy(i)=1-sum(t'-testY'~=0)/length(t);
end
subplot(1,2,2)
hist(accuracy)
title('accuracy distribution over 1000 forests with different data')

%% leave one out accuracy testing %%
for i=1:length(Y)
    trainX=[X(1:i-1,:);X(i+1:end,:)];
    trainY=[Y(1:i-1,:);Y(i+1:end,:)];

    testX=X(i,:);
    testY=Y(i,:);
    
    tree=train_decision_tree(trainX,trainY);
    
    t=classify_random_tree(testX,tree);

    accuracy(i)=1-sum(t'-testY'~=0)/length(t);
end

%% for two classes, find w with gradient descent %%
X=[(randn(100,2)+repmat([1 1],100,1))*[2 3;0 5];
(randn(70,2)+repmat([4 0.2],70,1))*[1 0;3 5]];

Y=[repmat(1,100,1);repmat(-1,70,1)];

subplot(1,2,1)
plot(X(1:100,1),X(1:100,2),'rx')
hold on
plot(X(101:end,1),X(101:end,2),'bx')

hold off

X=[ones(size(X,1),1) X];

w=Y'/X';

subplot(1,2,2)
plot(X(X*w'>0,2),X(X*w'>0,3),'ro')
hold on
plot(X(X*w'<=0,2),X(X*w'<=0,3),'bo')

%it's important that the first column of X is ones
total_entropy=classifier_entropy([1 0 0]',X,Y)

Q=total_entropy-classifier_entropy(w',X,Y)

hold off
