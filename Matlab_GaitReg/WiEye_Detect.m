function [UserDet,Confidence] = WiEye_Detect(PCANeed,FS, Classifier, ConfidenceThr)
%% Parameters
FeaCutoff = [0.7 10; 0.3 2]; % High freq; Active Freq
MinDist = 0.5; %s
%% Initialize
Num_PCA = size(PCANeed,1);
PCA1 = squeeze(PCANeed(1,:,1));
FeatureNum = 30;
%% Step Detection
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
%% Frequency domain feature calculation
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

%% Feature Extraction
Features = zeros(FeatureNum * 3 * Num_PCA, 0);
WalkDuration = SLen / FS;
Time = 0 : 1/FS : WalkDuration-1/FS;
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
    Features = cat(2,Features,Feature');
end
if isempty(Features) 
    UserDet = 0;
    Confidence = -10;
else
    [yfit,confidence,~] = predict(Classifier,Features');
    [Md,Fd] = mode(yfit);
    Confidence = mean(confidence(yfit == Md,Md));
%     disp(Confidence);
    if Fd > (length(yfit)/2) && ...
            Confidence > ConfidenceThr
            %sum(confidence(:,Md) > ConfidenceThr) > (length(yfit)/2)
        UserDet = Md;
    else
        UserDet = length(yfit) + 1;
    end
end