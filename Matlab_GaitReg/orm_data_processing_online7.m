% Use in WiEye.m
% version 6.0: you can combine any preprocessing method of the data.
% version 5.0: just run once and you can see the continous files.
% Please run this function first and then run log_file_online5 relevant
% Or It will RUN ERROR in order to prevent process the whole file
% CSI & magnitude change:
% ------------------------------+++
% CSI Read:                     |-|
% mag :                  1234...
% -----------------------+++++++++++++++++++++***
% | mag did not contain  | | change_plot_time | new data
%                        | plot_start_time
% changed mag:             1234...
% -------------------------++++++++++++++++++++++
%                          | plot_start_time
% 1. Intialize Variables/ parameters
% repeat
% 2. Read the file 'com' to get the file name
% 3. Loop the following codes until the file didn't be written
%   3.1 Copy the file and Read the file for the CSI raw data
%   3.2 Data processing
%   3.3 Draw figure
% go to 2.

% 1. Initialize variables
% Data processing 
    Process = [0    % 1. Multipath Removal
        1           % 2. Hampel Identifier
        1           % 3. Butterworth Low-pass Filter
        1           % 4. Principal Component Analysi (PCA)
        0           % 5. Spectrogram
        1           % 6. Log Total Time
        1];         % 7. FFT and walk detection
    WalkAntenna = 2;
    cutoff = [0.2 10];
    MRCutTime = 0.5;
    WalkDet = [3 8 3 9]; 
    % (Feq1, Feq2, Threshold, DetectStartTime)
    WalkTime = [2, 8]; % minimum and maximum time of walking event
    % the minimum time is the time window. 
    hampel_k = 120; % 200
    hampel_nsigma = 1.30;%1.30
    Num_PCA = 3;
    CutTime = 0.4;
% For Showing Data
    d_time = false;      % debug
    time_over = 5;      % Time out threshold: s
    antenna = [1 2 3];  % antenna
    max_plot_time = 6;  % Figure's x axis
    refresh_time = 0.8;
% communication file
    com_file = '/home/jennygroup/Matlab/com';
    ModelFile = '/home/jennygroup/Matlab/WiEyeMdl0528130946.mat';
% for walk detection
%     slen = max_plot_time / 2; %s,for walk detection 
%     Threshold = 3;
%     no_human_flag = 1;
%     slience_time = 5; %s, when activity detected, slience for 5s
%     no_act_time = 60; %s, when no activity perform for 1min,
% % d = fdesign.lowpass('N,F3db', 10, cutoff, 867);

% Error if file has already started
HaveClassifier = true;
try
    load(ModelFile);
catch ME
    if strcmp(ME.identifier, 'MATLAB:load:couldNotReadFile')
        disp(ME.message);
        warning('Orm: Do not have model!');
        HaveClassifier = false;
    end
end
in = fopen(com_file,'r');
file_name = fgetl(in);
fclose(in);
state = file_name(end);
if state == '1'
    error('ERROR: Please Run This Script First!');
end
% close all
% h = figure;
% set(h,'units','normalized','position',[0 0.38 0.70 0.58]);
disp('Ready to Begin!');
while 1 == 1 && Running
    state = '0';
    while state ~= '1' && Running
        in = fopen(com_file,'r');
        file_name = fgetl(in);
        fclose(in);
        state = file_name(end);
        pause(refresh_time);
    end
% intialize
    file_name(end - 1 : end) = [];
    file_name(1 : 7) = [];
    duration = refresh_time;        % refresh time
    plot_start_time = 1;            % used in plot()
    total_time = 0;                 % total time
    cursor_series = ones(1,6000);
    mag = zeros(30,0,3);
    total_len = 0;
    % File cursor
    % cursor_last = 0;
    cursor_current = 1;
    % Time Out
    time_out = 0;
    isWalking = false;
    Slience = true;
    WalkStarted = false;
    LogSt = total_time;
    LogRaw = zeros(30, 0, 3);
    while(time_out < time_over) && Running
        total_time = total_time + duration;
        cursor_last = cursor_current;
        tic
    % 3.1 Copy the file and Read the file for the CSI raw data
        % copyfile(file_name,'temp.dat','f');
        % Do not copy the file
        try 
            [CSI_trace,cursor_current] = read_bf_file_online(...
                file_name, cursor_last);
        catch ME
            warning('Error in read_bfee: Wrong beamforming matrix size.');
            rethrow(ME);
        end
        if cursor_last == cursor_current
            duration = toc;
            if (duration <= refresh_time)
                pause(refresh_time-duration); 
                duration = refresh_time;
            end
            time_out = time_out + duration;
            continue;
        else
            time_out = 0;
        end
        len_t = size(CSI_trace, 1);
        total_len = total_len + len_t;
        cursor_series(ceil(total_time)) = total_len;
        raw_data = zeros(30,len_t, 3);
        try
        for i = 1:len_t
            csi_entry = CSI_trace{i}; %for every packet
            csi = get_scaled_csi(csi_entry); 
            % csi(Ntx, Nrx, Channel) 2 * 3 * 30
            csi = squeeze(csi(1, :, :))'; 
            raw_data(:, i, :) = csi;
        end
        catch
            continue;
        end
        % for logging
        if Log
            if isempty(LogRaw)
                LogSt = total_time;
            end
            LogRaw = cat(2, LogRaw,raw_data);
        else
            if ~isempty(LogRaw)
                FNum = FNum + 1;
                NewFileName = sprintf('%02d.mat', FNum);
                WalkDuration = total_time - LogSt;
                save(fullfile(FileDir, NewFileName),...
                    'LogRaw','WalkDuration');
                set(handles.CountText,'String', sprintf('%02d', FNum));
                LogRaw = zeros(30,0,3);
            end
        end
    % 3.2 Data Processing
        %%%%%% 3.2.1 Multipath Removal
        if Process(1) == 1
            MR_ratio = MRCutTime/0.05335/30;
            % Total bandwith = 20MHz, 64 subcarriers, 30 collected;
            % Assume measured channel = 20MHz * 30 / 64 = 9.375MHz;
            % -9.375~9.375MHz then every interval of time domain is
            % 1/(9.375MHz*2) = 0.05535ms;
            csi_MR = zeros(size(raw_data));
            Start_t = round(size(raw_data,1) * MR_ratio);
            for i = 1 : size(raw_data,3)
                PDP  = ifft(squeeze(raw_data(:,:,i)));
                PDP(Start_t:end,:) = 0;
                csi_MR(:,:,i) = fft(PDP);
            end
            mag = cat(2,mag,abs(csi_MR));
        else
            mag = cat(2,mag,abs(raw_data));
        end
        if size(mag,2) < 200
            continue;
        end
    
        %%%%%% Change the mag size if the total time is lager than 25s
        if total_time > max_plot_time + 2 * refresh_time
            change_plot_start_time = ceil(total_time - max_plot_time);
            while change_plot_start_time > plot_start_time &&...
                    cursor_series(change_plot_start_time) == 1
                change_plot_start_time = change_plot_start_time - 1;
            end
            mag(:,1:cursor_series(change_plot_start_time)...
                -cursor_series(plot_start_time),:) = [];
            plot_start_time = change_plot_start_time;
        end
        len = size(mag,2);
        time = total_time - plot_start_time;
        sample_rate = len / time;
        
        %%%%%% 3.2.2 Hampel Identifier
        if Process(2) == 1
            outliered = zeros(30,len,3);
            for ham_i = 1:3
                if Process(4) == 0
                    [outliered(1,:,ham_i), Hampel_Idx] = hampel(...
                        mag(1,:,ham_i)',hampel_k,hampel_nsigma);
                else
                    for ham_j = 1 : 30
                        outliered(ham_j,:,ham_i) = hampel(...
                            mag(ham_j,:,ham_i)',hampel_k,hampel_nsigma);
                    end
                end
            end
        else
            outliered = mag;
        end
        
            
        %%%%%% 3.2.4 PCA
        if Process(4)==1 % adjust_process >= 3
            PCA1 = zeros(size(outliered,2),3); 
            PCANeed = zeros(Num_PCA,size(outliered,2),3);
            for an = 1 : 3
                input = squeeze(outliered(:,:,an));
                input = input';
                coeff = pca(input);
                PCA1(:,an) = input * coeff(:,1);
                tmp = input * coeff(:,1:Num_PCA);
                PCANeed(:,:,an) = tmp';
            end
        else
            PCA1 = squeeze(outliered(1,:,:));
            PCANeed = squeeze(outliered(1:Num_PCA,:,:));
%             all(PCA1(:,1)==PCA1(:,3))
        end
        %%%%%% 3.2.3 Butterworth low-pass filter
        if Process(3) == 1
%             d = fdesign.bandpass('N,F3dB1,F3dB2',8, cutoff(1),...
%                   cutoff(2), sample_rate);
%             hd = design(d, 'butter');
            if total_time  < (max_plot_time / 2) ||~exist('LPFilter','var')
                LowPass = fdesign.lowpass('N,F3db', 6,...
                    cutoff(2), 1700);
                LPFilter = design(LowPass,'butter');
            else
                if total_time < max_plot_time 
                    LowPass = fdesign.lowpass('N,F3db', 6,...
                        cutoff(2), sample_rate);
                    LPFilter = design(LowPass,'butter');
                end % else use LowPass calculated last time
            end
%             LPFilter = design(LowPass,'butter');
            filtered = zeros(Num_PCA, len,3);
            for i = 1:3
                if Process(4) == 0
                    filtered(1,:,i) = filter(LPFilter, PCANeed(1,:,i));
                else
                    for j = 1 : Num_PCA
                        filtered(j,:,i) = filter(LPFilter, PCANeed(j,:,i));
                        filtered(j,1:ceil(CutTime * sample_rate),i) = ...
                            filtered(j,ceil(CutTime * sample_rate),i);
                    end
                end
            end
        else
            filtered = PCANeed;
        end
        
        if total_time < max_plot_time + 2 * refresh_time
            inveral = total_time /len;
            Tidx = 0:inveral:(total_time - inveral);
        else
            inveral = (total_time - plot_start_time)/len;
            Tidx = plot_start_time:inveral:(total_time - inveral);
        end
        %%%%% 3.2.5 FFT 
        if Process(7) == 1
            Fs = sample_rate;
            T = 1 / Fs;
%             tt = (0:len-1)*T;
            FFTs = fft(PCA1);
            P2 = abs(FFTs/len);
            P1 = P2(1:floor(len/2) + 1);
            P1(2:end-1) = 2 * P1(2:end-1);
            FIdx = Fs * (0:floor(len/2))/len;
            WalkFeqIdx = find(FIdx > WalkDet(1) & FIdx < WalkDet(2));
            FFTEnergy = sum(P1(WalkFeqIdx));
            if total_time > WalkDet(4) && ~isWalking &&...
                    FFTEnergy > WalkDet(3)
                    %any(P1(WalkFeqIdx)> WalkDet(3))
                isWalking = true;
                WalkST = total_time;
                Slience = false;
            end
            if isWalking &&...
                    FFTEnergy < WalkDet(3)
                    %all(P1(WalkFeqIdx) < WalkDet(3))
                isWalking = false;
                if WalkStarted
                    WalkStarted = false;
                    disp('Walk ended!');
                end
            end
            if ~Slience && isWalking && ...
                    total_time - WalkST > WalkTime(1)
                Slience = true;
                SlienceTime = total_time;
                WalkStarted = true;
%                 disp('Walk started!');
                % Feature Extraction & Classification:
                if HaveClassifier
                    [UserD, Confidence] = WiEye_Detect(filtered,...
                        sample_rate, Classifier, ConfidenceThr);
                    if UserD == 0
                        fprintf('Unknown User.\n');
                    else
                        if UserD > length(Users)
                            fprintf('Other user! Ban!!\n');
                        else
                            fprintf('User %s Detected!\n',Users{UserD});
                        end
                    end
                    fprintf('Confidence: %f\n',Confidence);
                else
                    fprintf('.');
                end
            end
            if Slience && isWalking && ...
                    total_time - SlienceTime > WalkTime(1)
                Slience = false;
            end
            
        end
    % 3.3 Draw figure
        
        antenna_len = length(antenna);
        if ~exist('AxesHandle','var')
            AxesHandle = cell(4,1);
        end
        for ii = 1 : antenna_len
            AxesHandle{ii} = subplot(2,2,ii, 'Parent', FigH);
            plot(Tidx,PCA1(:,ii));
            if Process(4) == 0 && Process(2) == 1
                plot(Tidx,mag(1,:,ii));
                hold on
                plot(Tidx,PCA1(:,ii));
                plot(Tidx(Hampel_Idx), mag(1,Hampel_Idx,ii), 'sk',...
                    'MarkerSize',0.5);
                hold off
            end
            if Process(5)==1
                hold on;
                YLim = get(gca,'ylim');
                [~,w,Tidx,ps] = spectrogram(PCA1(:,ii),128,120,128,...
                    sample_rate,'yaxis');
                w = w + YLim(1);
                Drop = find(w > YLim(2));
                w(Drop) = [];
                ps(Drop,:) = [];
                h2 = surf(Tidx+time(1),w,10*log10(ps),'edgecolor','none');
                hold off;
                set(AxesHandle{ii},'child',[h1 h2]);
                colorbar('East'); % power spectral density
                ax2 = axes('Position',get(AxesHandle{ii},'Position'),...
                    'YAxisLocation','right','Color','none');
                YLim2 = [0 YLim(2) - YLim(1)];
                ax2.XAxis.Visible = 'off';
                set(ax2,'xlim', get(AxesHandle{ii},'xlim'),'ylim',YLim2);
                ylabel('Frequency/Hz');
                set(gcf,'Child',[AxesHandle{ii} ax2]);
            end
            axis tight
            Xlim = get(gca,'xlim');
            if Process(3) == 1
                Xlim(1) = Xlim(1) + (Xlim(2) - Xlim(1)) * 0.1;
                set(gca,'xlim',Xlim);
            end
%             temp = strfind(file_name,'/');
            temp = file_name(1:end);
            temp = strrep(temp,'\','\\');
            temp = strrep(temp,'_','\_');
            if ii == antenna_len
                title(['antenna: ' num2str(ii) ' file:' temp]);
            else
                title(['antenna: ' num2str(ii)]);
            end
            xlabel('time/s');
            ylabel('amplitude/dB');
        end
        if Process(7) == 1
            AxesHandle{4} = subplot(2,2,4);
%             Idxs = find(FIdx > WalkDet(1) & FIdx < (WalkDet(2) + 2));
            plot(FIdx(WalkFeqIdx), P1(WalkFeqIdx));
            hold on;
%             line([5 5],[0 FFTEnergy],...
%                 'LineWidth',3);
            hold off;
            set(AxesHandle{4},'YLim',[0 3]);
            xlabel('f (Hz)');
            ylabel('|P1(f)|');
            title(sprintf('Energy: %.3f',FFTEnergy));
        end
        drawnow;
        duration = toc;
        if (d_time)  || (duration > 1.5 * refresh_time)
            display(['duration:' num2str(duration)]);
        end
        if (duration <= refresh_time)
           pause(refresh_time-duration); 
           duration = refresh_time;
        end
%         fprintf('start_time: %f\n',plot_start_time);
    end
%     disp(['file was not written for' num2str(refresh_time) 's!...']);
    if Process(6) == 1
        temp = strfind(file_name,'/');
        log_dir = file_name(1:temp(end));
        log_file_name = fullfile(log_dir, 'duration.txt');
        out = fopen(log_file_name,'a');
        fprintf(out,'%s %s\n',log_file_name,num2str(total_time));
        fclose(out);
    end
    
    title('Ready');
    disp('---------------------------');
    feature_num = total_time;
end
disp('End showing data');
