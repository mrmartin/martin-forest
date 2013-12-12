function class = classify_random_tree(X,parent)
    %X must contain a dimension where every value is 1, and mustbe the same
    %number of dimensions as the training data
    
    %each datapoint of is a row!
    
    %if X has a lot of data points, run this tree for each of them
    if(size(X,1)>1)
        class=zeros(size(X,1),1);
        for i=1:size(X,1)
            class(i)=classify_random_tree(X(i,:),parent);
        end
    else
        if(isfield(parent,'w'))
            %if w*X>0.5, go right
            if((parent.w'*X')>0.5)
                %disp('right')
                class = classify_random_tree(X,parent.right);
            else
                %disp('left')
                class = classify_random_tree(X,parent.left);
            end
        else%if there are no children, return value
            class=parent.value;
        end
    end
end