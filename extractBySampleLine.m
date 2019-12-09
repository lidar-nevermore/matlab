function extractBySampleLine(inputFile,outputFile,leftTopRow,leftTopCol,rows,cols)
% 00000 leftTopRow=400;leftTopCol=13001; rows=5500;cols=4664;
% 17664 leftTopRow=400;leftTopCol=1; rows=5500;cols=7900;

%% Projection information
Proj=geotiffinfo(inputFile);
R=Proj.SpatialRef;
Xlim1=R.XWorldLimits(1)+(leftTopCol-1)*R.CellExtentInWorldX;
Xlim2=R.XWorldLimits(1)+(leftTopCol+cols-1)*R.CellExtentInWorldX;
Ylim1=R.YWorldLimits(2)-(leftTopRow+rows-1)*R.CellExtentInWorldY;
Ylim2=R.YWorldLimits(2)-(leftTopRow-1)*R.CellExtentInWorldY;
R2 = maprasterref(                  ...
'XWorldLimits',[Xlim1,Xlim2],...
'YWorldLimits',[Ylim1,Ylim2],...
'ColumnsStartFrom','north',...
'RasterSize', [rows cols]);

%% data
data=geotiffread(inputFile);
data=data(leftTopRow:leftTopRow+rows-1,leftTopCol:leftTopCol+cols-1);
% geotiffwrite('city-lulc-2.tif',data1,R2,'CoordRefSysCode','EPSG:32614');
geotiffwrite(outputFile,data,R2,'GeoKeyDirectoryTag',...
Proj.GeoTIFFTags.GeoKeyDirectoryTag,'TiffTags',struct('Compression',Tiff.Compression.LZW));
end
