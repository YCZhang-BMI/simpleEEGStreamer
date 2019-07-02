fwrite(ti2,num2str(count_sample));
count_sample =fread(ti2,1,'uint32');
if count_sample == 1
    Buffer =fread(ti2,[160 4000],'double')';
elseif count_sample~=1
    if count_sample-preCountSampleB >=4000 ||count_sample<preCountSampleB
        Buffer =fread(ti2,[160 4000],'double')';        
    elseif count_sample-preCountSampleB<4000
        Buffer(1:end-count_sample+preCountSampleB,:)=Buffer(1+count_sample-preCountSampleB:end,:);
        Buffer(end+1-count_sample+preCountSampleB:end,:)=fread(ti2,[160 (count_sample-preCountSampleB)],'double')';
    end
end
preCountSampleB = count_sample    ;
