function h=classifier_entropy(w,X,Y)
    %load iris.dat
    %X=iris;
    %Y=[repmat(1,50,1);repmat(2,50,1);repmat(3,50,1)];%categorical labels

    %w=rand(5,1);
    Yp=X*w;
    left=find(Yp>=0.5);
    right=find(Yp<0.5);

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
    h=-(rH+lH);
end