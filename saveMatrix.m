function saveMatrix( data,filename )
fileID = fopen(filename,'w');
formatSpec=[repmat('%f ',[1,size(data,2)-1]),'%f\n'];
fprintf(fileID,formatSpec,data');
fclose(fileID);
end
