function saveMatrix( data,filename )
fileID = fopen(filename,'w');
formatSpec=[];
for i=1:size(data,2)
    formatSpec=[formatSpec,'%f '];
end
formatSpec=[formatSpec,'\n'];
fprintf(fileID,formatSpec,data');
fclose(fileID);
end
