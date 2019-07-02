function [count_sample,data]=getNewData(preCountSample,ti2)

fwrite(ti2,num2str(preCountSample));
count_sample =fread(ti2,1,'uint32');
if count_sample == 1
    data =fread(ti2,[160 4000],'double')';
elseif count_sample~=1
    if count_sample-preCountSample >=4000 ||count_sample<preCountSample
        data =fread(ti2,[160 4000],'double')';        
    elseif count_sample-preCountSample<4000
        data=fread(ti2,[160 (count_sample-preCountSample)],'double')';
    end
end
end
