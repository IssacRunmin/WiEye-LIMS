% Using Logged RawData to build model
FileDirs = {
%     'Runmin_Ou';
    'Runmin_Ou1';
    'Yang_Xu';
    'Han_Fang';
    'Bo_Li';
    'Jiahua_Li';
    'Others'
};
% Others
UserLen = length(FileDirs);
%% Parameters
Process = [0    % 1. Multipath Removal
        1           % 2. Hampel Identifier
        1           % 3. Butterworth Low-pass Filter
        1           % 4. Principal Component Analysi (PCA)
        0           % 5. Spectrogram
        1           % 6. Log Total Time
        1];         % 7. FFT and walk detection
    Plot = [0 0 0 0 0 0 0];
Reprocess = false;
cutoff = [0.3 10];
FeaCutoff = [0.7 10; 0.3 2]; % High freq; Active Freq
MRCutTime = 0.5;
WalkDet = [0 5 23 9]; 
% (Feq1, Feq2, Threshold, DetectStartTime)
WalkTime = [2, 8]; % minimum and maximum time of walking event
% the minimum time is the time window. 
hampel_k = 120; % 200
hampel_nsigma = 1.30;%1.30
Num_PCA = 3;
MinDist = 0.5; % 0.5s, min distance for finding peaks
CutTime = 0.4; % 1s, cut the filtered data;
FeatureNum = 30;
TestRatio = 0.8;
%% Init
TrainData_All = cell(length(FileDirs),1);
TestData_All = cell(length(FileDirs),1);
for i = 1 : length(FileDirs)
    TrainData_All{i} = zeros(FeatureNum * 3 * Num_PCA, 0);
    TestData_All{i} = zeros(FeatureNum * 3 * Num_PCA, 0);
end
FeatureDir = fullfile('WiEye', 'TrainData');
if ~exist(FeatureDir, 'dir')
   mkdir(FeatureDir);
end
for User = 1 : UserLen
    Dirs = dir(fullfile('WiEye', FileDirs{User}, '*.mat'));
    disp(fullfile('WiEye', FileDirs{User}));
    FeatureFileName = fullfile(FeatureDir,[FileDirs{User} '.mat']);
    if exist(FeatureFileName, 'file') && ~Reprocess
        load(FeatureFileName);
        TrainData_All{User} = TrainData;
        TestData_All{User} = TestData;
        disp('Already Extract features');
        continue;
    end
    for File = 1 : length(Dirs)
        load(fullfile(Dirs(File).folder,Dirs(File).name));
%% Data Processing
        % 3.1 Multipath Removal
        FS = length(LogRaw) / WalkDuration;
        Time = 0 : 1/FS : WalkDuration-1/FS;
        if Process(1) == 1
            MR_ratio = MRCutTime/0.05335/30;
            % Total bandwith = 20MHz, 64 subcarriers, 30 collected;
            % Assume measured channel = 20MHz * 30 / 64 = 9.375MHz;
            % -9.375~9.375MHz then every interval of time domain is
            % 1/(9.375MHz*2) = 0.05535ms;
            csi_MR = zeros(size(LogRaw));
            Start_t = round(size(LogRaw,1) * MR_ratio);
            for i = 1 : size(LogRaw,3)
                PDP  = ifft(squeeze(LogRaw(:,:,i)));
                PDP(Start_t:end,:) = 0;
                csi_MR(:,:,i) = fft(PDP);
            end
            mag = abs(csi_MR);
        else
            mag = abs(LogRaw);
        end
        % 3.2 Hampel Identifier - Outlier removal
        len = length(LogRaw);
        if Process(2) == 1
            outliered = zeros(30,len,3);
            for ham_i = 1:3
                if Process(4) == 0
                    [outliered(1,:,ham_i), Hampel_Idx] = ...
                        hampel(mag(1,:,ham_i)',hampel_k,hampel_nsigma);
                else
                for ham_j = 1 : 30
                    outliered(ham_j,:,ham_i) = ...
                        hampel(mag(ham_j,:,ham_i)',hampel_k,hampel_nsigma);
                end
                end
            end
        else
            outliered = mag;
        end
        % 3.3 Butterworth low pass filter
        
        
        
        if Process(4)==1 % adjust_process >= 3
            PCANeed = zeros(Num_PCA,size(outliered,2),3); 
            for an = 1 : 3
                input = squeeze(outliered(:,:,an));
                input = input';
                coeff = pca(input);
                tmp = input * coeff(:,1 : Num_PCA);
                PCANeed(:,:,an) = tmp';
            end
            if Plot(4) == 1
            figure(4);
            plot(Time, outliered(1,:,1));
            hold on;
            plot(Time, PCANeed(1,:,1));
            hold off;
            set(gca,'FontSize',18,'FontName','Times');
            xlabel('\fontname{Song}\fontsize{18}时间/秒');
            ylabel('\fontname{Song}\fontsize{18}幅值');
            set(gcf,'Color','w');
            legend('去极值滤波信号','第一个主成分');
            end
        else
            PCANeed = outliered(1 : Num_PCA,:,:);
%             all(PCA1(:,1)==PCA1(:,3))
        end
        if Process(3) == 1
            LowPass = fdesign.lowpass('N,F3db', 6, cutoff(2), FS);
            LPFilter = design(LowPass,'butter');
            filtered = zeros(Num_PCA, len,3);
            for i = 1:3
                if Process(4) == 0
                    filtered(1,:,i) = filter(LPFilter, PCANeed(1,:,i));
                else
                    for j = 1 : Num_PCA
                        filtered(j,:,i) =filter(LPFilter,PCANeed(j,:,i));
                        filtered(j,1:ceil(CutTime * FS),i) = ...
                            filtered(j,ceil(CutTime * FS),i);
                    end
                end
            end
            if Plot(3) == 1
            figure(3);
            plot(Time, PCANeed(1,:,1));
            hold on;
            plot(Time, filtered(1,:,1),'LineWidth',2);
            hold off;
            set(gca,'FontSize',18,'FontName','Times');
            xlabel('\fontname{Song}\fontsize{18}时间/秒');
            ylabel('\fontname{Song}\fontsize{18}幅值');
            set(gcf,'Color','w');
            legend('第一个主成分','巴特沃斯滤波');
            end
        else
            filtered = PCANeed;
        end
        PCANeed = filtered;
        PCA1 = squeeze(filtered(1,:,1));
        [~,Locs] = findpeaks(100-PCA1,'MinPeakDistance', ceil(MinDist*FS));
        Thr = 0.5 * std(PCA1);
        Mean = mean(PCA1);
        for i = 1 : length(Locs)
            if abs(PCA1(Locs(i)) - Mean) > Thr
                StartI = i;
                break;
            end
        end
        Locs(1:StartI-1) = [];
        valleys = zeros(0,2);
        for i = 1 : length(Locs) - 1
            if abs(PCA1(Locs(i)) - Mean) > Thr && ...
                    abs(PCA1(Locs(i+1)) - Mean) > Thr
                valleys = cat(1,valleys, [Locs(i) Locs(i+1)]);
            end
        end
        % Frequency domain feature calculation
        LP = fdesign.lowpass('N,F3db', 6, FeaCutoff(1,1), FS);
        Filter1 = design(LP,'butter');
        BP = fdesign.bandpass('N,F3dB1,F3dB2',...
            6,FeaCutoff(1,1),FeaCutoff(1,2),FS);
        Filter2 = design(BP,'butter');
        BP = fdesign.bandpass('N,F3dB1,F3dB2',...
            6,FeaCutoff(2,1),FeaCutoff(2,2),FS);
        Filter3 = design(BP,'butter');
        PCAFilter = cell(3,1);
        FFTs = cell(3, 3, Num_PCA);
        NNs = cell(3, 3, Num_PCA);
        Energy = zeros(3, 3, Num_PCA);
        Entropy = zeros(3, 3, Num_PCA);
        ComFreqRatio = zeros(3, 3, Num_PCA);
        SLen = size(PCANeed,2);
        PCAFilter{1} = zeros(size(PCANeed));
        PCAFilter{2} = zeros(size(PCANeed));
        PCAFilter{3} = zeros(size(PCANeed));
        for an = 1 : 3
            for p = 1 : Num_PCA
                PCAFilter{1}(p,:,an) = filter(Filter1, PCANeed(p,:,an));
                PCAFilter{2}(p,:,an) = filter(Filter2, PCANeed(p,:,an));
                PCAFilter{3}(p,:,an) = filter(Filter3, PCANeed(p,:,an));
                for i = 1 : 3
                    tmp = fft(PCAFilter{i}(p,:,an));
                    tmp = abs(tmp / SLen);
                    tmp = tmp(1:floor(SLen/2)+1);
                    tmp(2:end-1) = 2*tmp(2:end-1);
                    FFTs{i, an, p} = tmp;
                    NNs{i, an, p} = tmp ./ sum(tmp);
                    Energy(i, an, p) = sum(tmp.^2);
                    Entropy(i, an, p) = ...
                        (-sum(NNs{1}.*log2(NNs{1})))/log(FS/2);
                    ComFreqRatio(i, an, p) = max(tmp)/sum(tmp);
                end
            end
        end
        
        % Feature Extraction
        Feature_tmp = zeros(FeatureNum * 3 * Num_PCA, 0);
        for step = 1 : size(valleys,1)
            Feature = zeros(1,FeatureNum*3*Num_PCA);
            for an = 1 : 3
            for p = 1 : Num_PCA
                MIdx = valleys(step,1):valleys(step,2);
                Wave = filter(Filter2,squeeze(...
                    PCANeed(p,MIdx,an)));
                WMean = Wave - mean(Wave); % The number cross Mean
                WLen = length(Wave);
                QRatio = [25 50 75];
                Y_EY = Time(MIdx) - mean(Time(MIdx));
                CovMatrix = cov(Wave,Y_EY);
                CCMatrix = corrcoef(Wave,Y_EY); % Correlation coefficients
                idx = ((an-1) * Num_PCA + p - 1) * FeatureNum;
                % Features: 
                % CV: Coefficient of variation
                % IQR: Inter Quartile Range
                % TNI: Trapezoidal numerical integration
                % SM: 
                Feature(idx+ 1) = min(Wave); % minimum
                Feature(idx+ 2) = max(Wave); % maximum
                Feature(idx+ 3) = prctile(Wave,90); % max90th
                Feature(idx+ 4) = prctile(Wave,10); % min90th
                Feature(idx+ 5) = mean(Wave); % Mean
                Feature(idx+ 6) = std(Wave); % Stander diviation
                Feature(idx+ 7) = var(Wave)/10; % Varriation, std^2
                Feature(idx+ 8) = Feature(idx+2) - Feature(idx+1); %Range
                Feature(idx+ 9) = Feature(idx+6)/(Feature(idx+5)*100);% CV
                Feature(idx+10) = skewness(Wave); % skewness(E(X^3))
                Feature(idx+11) = kurtosis(Wave); % kurtosis(E(X^4))
                Feature(idx+12) = prctile(Wave,25); % Q1(1st Quartile)
                Feature(idx+13) = prctile(Wave,75); % Q3(3rd Quartile)
                Feature(idx+14) = prctile(Wave,50); % middle
                Feature(idx+15) = Feature(idx+13) - Feature(idx+12); % IQR
                Feature(idx+16) = sum(WMean(1:end-1).* WMean(2:end) < 0); 
                Feature(idx+17) = trapz(1:WLen,Wave)/1000; % TNI/1000
                Feature(idx+18) = QRatio/100 * prctile(Wave,QRatio)'; % SM
                Feature(idx+19) = mean(Wave .^ 2)/10; % E(X^2);
                Feature(idx+20) = CovMatrix(1,2); % Cov(act, time)
                Feature(idx+21) = CCMatrix(1,2); % Correlation coefficients
                Feature(idx+22) = Energy(1, an, p)/100; % Energy Low band
                Feature(idx+23) = Energy(2, an, p); % Energy Active band
                Feature(idx+24) = Energy(3, an, p); % Energy High band
                Feature(idx+25) = Entropy(1, an, p); % Entropy LB
                Feature(idx+26) = Entropy(2, an, p); % Entropy AB
                Feature(idx+27) = Entropy(3, an, p); % Entropy HB
                Feature(idx+28) = ComFreqRatio(1, an, p); % DomFreqRatio LB
                Feature(idx+29) = ComFreqRatio(2, an, p); % DomFreqRatio AB
                Feature(idx+30) = ComFreqRatio(3, an, p); % DomFreqRatio HB
            end
            end
%             TrainData_All{User} = cat(2,TrainData_All{User},Feature');
            Feature_tmp = cat(2, Feature_tmp, Feature');
        end
        fprintf('%s: %d steps\n', Dirs(File).name, size(valleys,1));
        if File < length(Dirs) * TestRatio
            TrainData_All{User} = cat(2,TrainData_All{User}, Feature_tmp);
        else
            TestData_All{User} = cat(2,TestData_All{User}, Feature_tmp);
        end
    % Draw figure
        if Plot(1) == 1
            figure(1);
            plot(Time,mag(1,:,1));
            set(gca,'FontSize',18,'FontName','Times');
            xlabel('\fontname{Song}\fontsize{18}时间/秒');
            ylabel('\fontname{Song}\fontsize{18}幅值');
            set(gcf,'Color','w');
        end
        if Plot(2) == 1
            figure(2);
            plot(Time, mag(1,:,1));
            hold on;
            plot(Time, outliered(1,:,1));
            hold off;
            set(gca,'FontSize',18,'FontName','Times');
            xlabel('\fontname{Song}\fontsize{18}时间/秒');
            ylabel('\fontname{Song}\fontsize{18}幅值');
            set(gcf,'Color','w');
            legend('原始幅值','去极值滤波信号');
        end
    end
    % will skip , do not code here
    TrainData = TrainData_All{User};
    TestData = TestData_All{User};
    save(FeatureFileName,'TrainData','TestData');
    
end
% Create model:
NFold = 10;
RandomData = false;
if strcmp(FileDirs{end},'Others')
    OtherData = TrainData_All{end};
    TrainData_All(end) = [];
    UserLen = UserLen - 1;
    
end

% Initialize
Label = cell(UserLen,1);
NumSampleGroup = zeros(NFold, UserLen);
ValidationAcc = zeros(NFold,1);
ValidationCM = zeros(UserLen,UserLen,NFold);
start_indices = ones(1, UserLen);
VaConfidence = cell(UserLen,1);

for u = 1 : UserLen
    VaConfidence{u} = zeros(0,UserLen);
    steps = size(TrainData_All{u},2);
    Label{u} = ones(1,steps);
    Remainder = rem(steps, NFold);
    Quotient = (steps - Remainder) / NFold;
    NumSampleGroup(:,u) = ones(NFold,1) * Quotient;
    NumSampleGroup(1:Remainder,u) = NumSampleGroup(1:Remainder,u) + 1;
    if RandomData
        perm = randperm(steps);
        TrainData_All{u} = TrainData_All{u}(:,perm);
    end
end
for Round = 1 : NFold
    x_test = [];
    y_test = [];
    x_train = [];
    y_train = [];
    for u = 1 : UserLen
        samples = TrainData_All{u}';
        s = start_indices(u);
        t = s + NumSampleGroup(Round, u) - 1;
        x_test = cat(1,x_test,samples(s:t,:));
        y_test = cat(1,y_test,u * ones(t - s + 1,1));
        samples(s:t,:) = [];
        x_train= cat(1,x_train,samples);
        y_train = cat(1,y_train,u * ones(size(samples, 1),1));
        start_indices(u) = t + 1;
    end
    if Round == 0
        ClassifierTemplate = fitcecoc(x_train,y_train,...
            'OptimizeHyperparameters','all',...
            'HyperparameterOptimizationOptions',...
            struct('AcquisitionFunctionName',...
            'expected-improvement-plus','MaxObjectiveEvaluations',60,...
            'UseParallel', true, 'Kfold',10,'Verbose',1,'ShowPlots',true));
        Learner = ClassifierTemplate.ModelParameters.BinaryLearners;
    end
    Classifier = fitcecoc(x_train, y_train, 'Coding',...
                ClassifierTemplate.CodingName, 'Learners',Learner);
    [yfit,confidence,~] = predict(Classifier,x_test);
    ValidationAcc(Round) = sum(yfit==y_test)/length(yfit);
    for u = 1 : UserLen
        VaConfidence{u} = cat(1,VaConfidence{u}, confidence(y_test==u,:));
    end
    fprintf('Round #%2d: %.3f%%\n',Round, ValidationAcc(Round) * 100);
    for i = 1 : length(y_test)
        ValidationCM(y_test(i), yfit(i), Round) = ...
            ValidationCM(y_test(i), yfit(i), Round) + 1;
    end
end
% Test Data
TestData = TestData_All{1};
YTest = ones(1,size(TestData,2));
Confidence = cell(UserLen,1);
ConfusionMat = zeros(UserLen);
for i = 2 : UserLen
    TestData = cat(2,TestData, TestData_All{i});
    YTest = cat(2, YTest, ones(1, size(TestData_All{i},2)) .* i);
end
YTest = YTest';
[YFit, TConfidence, ~] = predict(Classifier,TestData');
Accuracy = sum(YFit == YTest)/length(YFit);
fprintf('Round Validate Acc: %.3f%%\n', mean(ValidationAcc) * 100);
% fprintf('Round #Test: %.3f\n',Accuracy * 100);
for u = 1 : UserLen
    Confidence{u} = TConfidence(YTest==u,:);
end
for i = 1 : length(YFit)
    ConfusionMat(YTest(i), YFit(i)) = ConfusionMat(YTest(i), YFit(i)) + 1;
end

% Need to calculate confidence.
if strcmp(FileDirs{end},'Others')
    [yfit, BandCon, ~] = predict(Classifier,OtherData');
    tmp = max(BandCon,[],2);
    ConfidenceThr = prctile(tmp,90);
    FileDirs(end) = [];
else
    ConfidenceThr = -0.1;
end
Users = FileDirs;
SaveTime = datestr(datetime(),'mmddHHMMSS');
save(['WiEyeMdl' SaveTime '.mat'],'Classifier','Users','Process',...
    'cutoff','FeaCutoff','MRCutTime','WalkTime','hampel_k',...
    'hampel_nsigma','Num_PCA','MinDist','CutTime','FeatureNum',...
    'ConfidenceThr','Accuracy','BandCon','ClassifierTemplate',...
    'Confidence','ConfusionMat');