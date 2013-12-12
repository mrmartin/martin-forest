function v = mymedian(v)
    u=unique(v);
    [~,i]=max(histc(v,u));
    v=u(i);
end