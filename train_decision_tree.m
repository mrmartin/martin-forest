function parent = train_decision_tree(X,Y,varargin)
    if(length(Y)==0 || size(X,1)~=size(Y,1))
        disp('cannot make classifier')
        r(1)
    end
    if(nargin>=3)
        parent=varargin{1};
    else
        parent=struct;
    end
    %determine if this is root, and if yes then train it here
    if(~isfield(parent,'w'))
        un=unique(Y);
        %divide n classes into 2 subsets in all possible ways
        p=SetPartition(length(un),2);
        H=zeros(size(p));
        for i=1:size(p,1)
            %train a linear classifier for each, and calculate the resulting entropy
            t=zeros(size(Y));
            t(ismember(Y,un(p{i}{1})))=1;
            %t(Y==p{i}{2})=0;
            w=(t'/X')';
            H(i)=classifier_entropy(w,X,Y);
        end

        %chose classifier which most decreased entropy
        [~,i]=min(H);
        t=zeros(size(Y));
        t(ismember(Y,un(p{i}{1})))=1;
        %t(Y==p{i}{2})=0;
        w=(t'/X')';
        parent.w=w;
    end
    
    ce=classifier_entropy(parent.w,X,Y);
    %make left classifier
    
    Xl=X(X*parent.w<0.5,:);
    Yl=Y(X*parent.w<0.5,:);    
    %disp(['Yl has ' num2str(length(Yl)) ' datapoints']);
    ul=unique(Yl);

    left=struct;
    if(length(unique(Yl))>1)%if there's more than one class
        %divide n classes into 2 subsets in all possible ways
        p=SetPartition(length(ul),2);
        H=zeros(size(p));
        for i=1:size(p,1)
            %train a linear classifier for each, and calculate the resulting entropy
            t=zeros(size(Yl));
            t(ismember(Yl,ul(p{i}{1})))=1;
            %t(Y==p{i}{2})=0;
            w=(t'/Xl')';
            H(i)=classifier_entropy(w,Xl,Yl);
        end

        %chose classifier which most decreased entropy
        [~,i]=min(H);
        
        t=zeros(size(Yl));
        t(ismember(Yl,ul(p{i}{1})))=1;
        w=(t'/Xl')';
        %if all points are classified in the same class, that's bad
        if(length(unique(Xl*w<0.5))>1)
            %it's very important that even if there are mistakes, there
            %are some correct classifications in both classes.
            left.w=w;
            %train the leaves
            left=train_decision_tree(Xl,Yl,left);
        else
            %randomly remove points from the more numerous class until
            %there are some correct classifications in both classes.
            H=H(i);
            Ys=Yl;
            Xs=Xl;
            while(length(unique(Xl*w<0.5))<2)
                %datapoints which are of the most numerous class
                numerous_elems=find(Ys==mymedian(Ys));
                %remove a random element from the most numerous class
                newset=setdiff(1:length(Ys),numerous_elems(ceil(rand*length(numerous_elems))));
                Xs=Xs(newset,:);
                Ys=Ys(newset);
                us=unique(Ys);
                p=SetPartition(length(us),2);
                H=zeros(size(p));
                for i=1:size(p,1)
                    %train a linear classifier for each, and calculate the resulting entropy
                    t=zeros(size(Ys));
                    t(ismember(Ys,us(p{i}{1})))=1;
                    %t(Y==p{i}{2})=0;
                    w=(t'/Xs')';
                    H(i)=classifier_entropy(w,Xs,Ys);
                end

                %chose classifier which most decreased entropy
                [~,i]=min(H);
                H=H(i);
            end
            t=zeros(size(Ys));
            t(ismember(Ys,us(p{i}{1})))=1;
            w=(t'/Xs')';
            left.w=w;
            left=train_decision_tree(Xl,Yl,left);
        end
    else
        left.value=unique(Yl);
        if(length(unique(Yl))==0)
            disp('WTF?');
            Yl
            r(1)
        end
        %no leaves to train
    end

    %make right classifier
    Xr=X(X*parent.w>=0.5,:);
    Yr=Y(X*parent.w>=0.5,:);
    ur=unique(Yr);
    %disp(['Yr has ' num2str(length(Yr)) ' datapoints']);

    right=struct;
    %[Xr Yr Xr*parent.w]
    %length(unique(Yr))
    if(length(unique(Yr))>1)%if there's more than one class
        %divide n classes into 2 subsets in all possible ways
        p=SetPartition(length(ur),2);
        H=zeros(size(p));
        for i=1:size(p,1)
            %train a linear classifier for each, and calculate the resulting entropy
            t=zeros(size(Yr));
            t(ismember(Yr,ur(p{i}{1})))=1;
            %t(Y==p{i}{2})=0;
            w=(t'/Xr')';
            H(i)=classifier_entropy(w,Xr,Yr);
        end

        %chose classifier which most decreased entropy
        [~,i]=min(H);
        %if the entropy isn't decreased, stop trying. A linear classifier
        %won't work here
        t=zeros(size(Yr));
        t(ismember(Yr,ur(p{i}{1})))=1;
        w=(t'/Xr')';
        if(length(unique(Xr*w<0.5))>1)            
            right.w=w;
            %train the leaves
            right=train_decision_tree(Xr,Yr,right);
        else
            %randomly remove points from the more numerous class until
            %points are classified into each class
            H=H(i);
            Ys=Yr;
            Xs=Xr;
            while(length(unique(Xr*w<0.5))<2)
                %datapoints which are of the most numerous class
                numerous_elems=find(Ys==mymedian(Ys));
                %remove a random element from the most numerous class
                newset=setdiff(1:length(Ys),numerous_elems(ceil(rand*length(numerous_elems))));
                Xs=Xs(newset,:);
                Ys=Ys(newset);
                us=unique(Ys);
                p=SetPartition(length(us),2);
                H=zeros(size(p));
                for i=1:size(p,1)
                    %train a linear classifier for each, and calculate the resulting entropy
                    t=zeros(size(Ys));
                    t(ismember(Ys,us(p{i}{1})))=1;
                    %t(Y==p{i}{2})=0;
                    w=(t'/Xs')';
                    H(i)=classifier_entropy(w,Xs,Ys);
                end

                %chose classifier which most decreased entropy
                [~,i]=min(H);
                H=H(i);
            end
            t=zeros(size(Ys));
            t(ismember(Ys,us(p{i}{1})))=1;
            w=(t'/Xs')';
            right.w=w;
            right=train_decision_tree(Xr,Yr,right);
        end
    else
        right.value=unique(Yr);
        if(length(unique(Yr))==0)
            disp('WTF?');
            Yr
            r(1)
        end
        %no leaves to train
    end
    parent.left=left;
    parent.right=right;
end