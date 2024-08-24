%% EXPLANATION
%{

The program takes and processes a picture each 2 seconds.
The human starts moving a green cube, the code has to detect its location in the chessboard.
Then the robot moves a red cube, but Arduino already knows its location, so there is no need to detect it.
Then the human moves again, and the code has to detect again this green cube, forgetting about the previous red cube.
And so on and so forth...

%}

%% START

% Set-up Serial Comunication
if ~isempty(instrfind)
     fclose(instrfind);
      delete(instrfind);
end

% Set-up Arduino and Webcam
cam = webcam('Trust Webcam');
arduino=serial('COM3','BAUD', 9600);
fopen(arduino);
pause(3);

% Take a photo of the empty chessboard to start
imm_old = snapshot(cam);    
imm_old = imm_old(5:475, 60:620, :);
imm_old = imrotate(imm_old, 180);
g_old = rgb2gray(imm_old);
[M,N] = size(g_old);

cont=0;
while cont < 4

x=0;
y=0;

start = fscanf(arduino,'%d'); % Arduino communicates when it is ready

if start == 1
    
while x == 0 && y == 0
 
imm_new = snapshot(cam);
imm_new = imm_new(5:475, 60:620, :);  % I look only the chessboard
imm_new = imrotate(imm_new, 180);
g_new = rgb2gray(imm_new);

figure(1); imshow([imm_old , imm_new]);
%figure(2); imshow([g_old , g_new]);

%% FILTERING & MORPHOLOGY

%Average filter
n=4;
h = ones(n , n)/(n*n);
f_old = imfilter(g_old,h);
f_new = imfilter(g_new,h);

% Resize the image so that morphological operators are faster
resize_value = 0.3;
f_old = imresize(f_old , resize_value );   
f_new = imresize( f_new , resize_value );
%figure(3); imshow([f_old , f_new]);

%Make the difference between current image and previous image to detect changes
image = uint8(abs(int16(f_new) - int16(f_old))) > 30;   
siluette = uint8( 255 * double( image ));
%figure(4); imshow(siluette);

%Closing 
mask  = strel('disk',1); 
fBinClose = imclose(siluette,mask);

%Opening 
mask  = strel('disk',1); 
fBinClose = imopen(fBinClose,mask);

figure(6); imshow(fBinClose);


%% ETIQUETTES

fEtiqs = bwlabel(fBinClose);
nEtiqs = max(max(fEtiqs));    % I calculate the numbers of objects detected

%% CENTROIDS

if ( nEtiqs < 10 ) % I go on only if I detect only 1 or 2 new objects, other cases are wrong. But I use 10 for robustness.
    
data = regionprops(fEtiqs); % I extrapolate the data
centro = cat( 1 ,  data.Centroid );   % I transform it in array
area = cat( 1 ,  data.Area );  % I transform it in array

%area_rojas(i)  % area dell'oggetto i-esimo
%centro_rojas(i,1)  % numero sx : oggetto i-esimo          numero dx : coordinata x
%centro_rojas(i,2)  % numero sx : oggetto i-esimo          numero dx : coordinata y

% I calculate only the centroids with a specified section and color
for i=1:nEtiqs
    if ( right_area( area(i) ) && right_color( imresize( imm_new, resize_value ) , fEtiqs, i ) )  
         x = (1/resize_value)*centro(i,1);   % I used resize, I have to come back to precedent coordinates
         y = (1/resize_value)*centro(i,2);
     end
end

end

% If I haven't detected centroids, wait 2 seconds and start again
if x==0 && y==0  
pause(2);   
end

end
    
imm_old = imm_new;   % I update the old version of the immage
g_old = g_new;  

[ move ] = position(x,y,M,N)  % It returns the number of the slot ( 1 - 9 ) where the new cube is located.
fprintf(arduino, move);  % Communicate to Arduino the slot

% I show the final centroid on the original image 
figure(7); imshow(imm_new)
hold on
plot( x, y, 'rx', 'LineWidth',8, 'MarkerSize',18  )
hold off

cont = cont+1;
end

pause(2)
start=0;
end

pause(5)
clear cam;


%% FUNCTIONS


function right = right_color( f , fEtiqs, k ) 

[M,N,P] = size(f);
cont_green = 0;
cont_tot = 0;

fRojo  = f(:,:,1);
fVerde = f(:,:,2);
fAzul  = f(:,:,3);

% Usa comando : colorThresholder

for i = 1:M
    for j = 1:N
        if ( fEtiqs(i,j) == k )  % I consider only the pixels with etiquette K
            cont_tot=cont_tot+1;
            if ( fRojo(i,j)<90 & fVerde(i,j)>45 & fVerde(i,j)<245 ) % I check if the color of the pixel is green
                cont_green=cont_green+1;
            end
        end
        
    end
end

percentage_green = cont_green/cont_tot;

right = percentage_green > 0.4;

end


function right = right_area( area )

area_cubo = 900;

right = ( area > 0.65*area_cubo) && (area < 1.35*area_cubo ); 

end


function [ move ] = position(x,y,M,N)
   
row = floor( (y*3)/M );      % 0 1 2
column = floor( (x*3)/N );   % 0 1 2

move = row*3 + column + 1;   % from 1 to 9


%     (0,0) ___________ x , N
%          |
%          |
%          |
%          |
%     M , y

% y : M = ? : 3   -->  row
% x : N = ? : 3   -->  column
   
end










%% EXPLANATION
%{
The program captures and processes an image every 2 seconds.
The human starts moving a green cube; the code needs to detect its location on the chessboard.
Then the robot moves a red cube, but Arduino already knows its location, so there is no need to detect it.
The human moves again, and the code needs to detect the green cube again, ignoring the previous red cube.
And so on and so forth...
%}

%% START

% Set-up Serial Communication
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

% Set-up Arduino and Webcam
cam = webcam('Trust Webcam');
arduino = serial('COM3', 'BAUD', 9600);
fopen(arduino);
pause(3);

% Capture a photo of the empty chessboard to start
imm_old = snapshot(cam);    
imm_old = imm_old(5:475, 60:620, :);
imm_old = imrotate(imm_old, 180);
g_old = rgb2gray(imm_old);
[M, N] = size(g_old);

cont = 0;
while cont < 4

    x = 0;
    y = 0;

    start = fscanf(arduino, '%d'); % Arduino communicates when it is ready

    if start == 1
    
        while x == 0 && y == 0
     
            imm_new = snapshot(cam);
            imm_new = imm_new(5:475, 60:620, :);  % Only look at the chessboard
            imm_new = imrotate(imm_new, 180);
            g_new = rgb2gray(imm_new);

            figure(1); imshow([imm_old, imm_new]);
            %figure(2); imshow([g_old, g_new]);

            %% FILTERING & MORPHOLOGY

            % Average filter
            n = 4;
            h = ones(n, n) / (n * n);
            f_old = imfilter(g_old, h);
            f_new = imfilter(g_new, h);

            % Resize the image so that morphological operators are faster
            resize_value = 0.3;
            f_old = imresize(f_old, resize_value);   
            f_new = imresize(f_new, resize_value);
            %figure(3); imshow([f_old, f_new]);

            % Difference between current image and previous image to detect changes
            image = uint8(abs(int16(f_new) - int16(f_old))) > 30;   
            silhouette = uint8(255 * double(image));
            %figure(4); imshow(silhouette);

            % Closing 
            mask = strel('disk', 1); 
            fBinClose = imclose(silhouette, mask);

            % Opening 
            mask = strel('disk', 1); 
            fBinClose = imopen(fBinClose, mask);

            figure(6); imshow(fBinClose);

            %% LABELING

            labeledImage = bwlabel(fBinClose);
            numObjects = max(max(labeledImage));    % Calculate the number of detected objects

            %% CENTROIDS

            if (numObjects < 10) % Continue only if 1 or 2 new objects are detected, use 10 for robustness.
                
                data = regionprops(labeledImage); % Extract data
                centroids = cat(1, data.Centroid);   % Convert to array
                area = cat(1, data.Area);  % Convert to array

                % Calculate only centroids with a specified area and color
                for i = 1:numObjects
                    if (correct_area(area(i)) && correct_color(imresize(imm_new, resize_value), labeledImage, i))  
                        x = (1 / resize_value) * centroids(i, 1);   % Convert back to original coordinates
                        y = (1 / resize_value) * centroids(i, 2);
                    end
                end

            end

            % If no centroids detected, wait 2 seconds and try again
            if x == 0 && y == 0  
                pause(2);   
            end

        end
    
        imm_old = imm_new;   % Update the old version of the image
        g_old = g_new;  

        [move] = position(x, y, M, N);  % Returns the number of the slot (1 - 9) where the new cube is located.
        fprintf(arduino, move);  % Communicate the slot number to Arduino

        % Display the final centroid on the original image 
        figure(7); imshow(imm_new);
        hold on;
        plot(x, y, 'rx', 'LineWidth', 8, 'MarkerSize', 18);
        hold off;

        cont = cont + 1;
    end

    pause(2);
    start = 0;
end

pause(5);
clear cam;

%% FUNCTIONS

function correct = correct_color(image, labeledImage, k) 

    [M, N, ~] = size(image);
    green_count = 0;
    total_count = 0;

    redChannel = image(:, :, 1);
    greenChannel = image(:, :, 2);
    blueChannel = image(:, :, 3);

    % Check color threshold
    for i = 1:M
        for j = 1:N
            if (labeledImage(i, j) == k)  % Consider only pixels with label k
                total_count = total_count + 1;
                if (redChannel(i, j) < 90 && greenChannel(i, j) > 45 && greenChannel(i, j) < 245) % Check if the color of the pixel is green
                    green_count = green_count + 1;
                end
            end
        end
    end

    green_percentage = green_count / total_count;

    correct = green_percentage > 0.4;
end


function correct = correct_area(area)

    cube_area = 900;

    correct = (area > 0.65 * cube_area) && (area < 1.35 * cube_area); 
end


function [move] = position(x, y, M, N)
   
    row = floor((y * 3) / M);      % 0 1 2
    column = floor((x * 3) / N);   % 0 1 2

    move = row * 3 + column + 1;   % From 1 to 9

    %     (0,0) ___________ x , N
    %          |
    %          |
    %          |
    %          |
    %     M , y

    % y : M = ? : 3   -->  row
    % x : N = ? : 3   -->  column
   
end
