FeatureNum = 30;
Data = TrainData_All{1};
Yfit = ones(1,size(TrainData_All{1},2));
for i = 2 : UserLen
    Yfit = cat(2,Yfit, ones(1,size(TrainData_All{i},2)) .* i);
end
for i = 2 : UserLen
    Data = cat(2,Data,TrainData_All{i}); 
end
Gains = zeros(1, FeatureNum * 9);
for i = 1 : FeatureNum * 9
    try
    Gains(i) = information_gain(Data(i,:)',Yfit');
    catch
        disp([num2str(i) 'Failed']);
    end
end
Gain = reshape(Gains,[FeatureNum,9]);
Gain_mean = mean(Gain,2);
[GainSort, Idx] = sort(Gain_mean,'descend');

Features = {
'Minimum';
'Maximum';
'Max90th';
'Min90th';
'Mean';
'Std';
'Var';
'Range';
'CV';
'Skewness';
'Kurtosis';
'Q1';
'Q3';
'Middle';
'IQR';
'CrossMean';
'TNI';
'SM';
'2^{nd} Moment';
'Cov';
'Correlation';
'LB Energy';
'AB Energy';
'HB Energy';
'LB Entropy';
'AB Entropy';
'HB Entropy';
'LB DomFreqRatio';
'AB DomFreqRatio';
'HB DomFreqRatio'};
FinalFeature = Features(Idx);
Fig = figure;
PlotN = 10;
GainPlot = GainSort(1:PlotN);
bar(GainPlot);
xlim([0 length(GainPlot) + 1]);
set(gca, 'ygrid','on','GridLineStyle','--');
set(gca, 'XTickLabel',FinalFeature(1:PlotN));
xtickangle(45);
% xlabel('\bf\fontname{Times}\fontsize{18}User');%写上对应的横纵坐标
ylabel('\bf\fontname{Times}\fontsize{18}IG Value');%写上对应的横纵坐标
set(gca,'FontSize',14,'FontName','Times');
set(gcf,'Color','w');%设置窗口的底纹为白色
[im_hatch,colorlist] = applyhatch_pluscolor(gcf,'////',0,0,[],120);

% % Total H(x)
% Size = zeros(1,3);
% Size(1) = size(PredictLabelTotal{1},1);
% Size(2) = size(PredictLabelTotal{2},1);
% Size(3) = size(PredictLabelTotal{3},1);
% temp = Size(1) / sum(Size);
% ES = temp*log(temp);
% temp = sum(Size(2:3))/sum(Size);
% ES = ES + temp * log(temp);
% ES = -ES;
% % For Simple Classifier
% Bound = Size(1);
% TrainDataA = cell(ClassifierNum,1);
% for i = 1 : ClassifierNum
%     TrainDataA{i} = cat(1,TrainData{i,1},TrainData{i,2},TrainData{i,3});
% end
% SimpleIG = zeros(FeatureNum * length(SimpleClassifier),1);
% i = 1;
% for j = 1 : length(SimpleClassifier)
%     Class = SimpleClassifier(j);
%     for k = 1 : FeatureNum
%         Mean = mean(TrainDataA{Class}(:,k));
%         Logic = TrainDataA{Class}(:,k) > Mean;
%         Sum = length(Logic);
%         Sum1 = sum(Logic);
%         Pos1 = sum(Logic(1 : Bound));
%         Neg1 = sum(Logic(Bound+1 : end));
%         EG1 = -(Pos1/Sum1) * log(Pos1/Sum1)- Neg1/Sum1 * log(Neg1/Sum1);
%         Sum2 = Sum - Sum1;
%         Pos2 = Bound - Pos1;
%         Neg2 = Sum - Bound - Neg1;
%         EG2 = - (Pos2/Sum2) * log(Pos2/Sum2) - Neg2/Sum2 * log(Neg2/Sum2);
%         SimpleIG(i) = ES - Sum1/Sum * EG1 - Sum2/Sum * EG2;
%         i = i + 1;
%     end
% end
% AdvancedIG_t = reshape(SimpleIG,FeatureNum,length(SimpleClassifier));
% [IG_SS,I_SS] = sort(SimpleIG,'descend');
% [IG_SSF,I_SSF] = sort(mean(AdvancedIG_t,2),'descend');
% % For Advanced Classifier
% Bound = Size(2);
% TrainDataA = cell(ClassifierNum,1);
% for i = 1 : ClassifierNum
%     TrainDataA{i} = cat(1,TrainData{i,2},TrainData{i,3});
% end
% AdvancedIG = zeros(FeatureNum * length(AdvancedClassifier),1);
% i = 1;
% for j = 1 : length(AdvancedClassifier)
%     Class = AdvancedClassifier(j);
%     for k = 1 : FeatureNum
%         Mean = mean(TrainDataA{Class}(:,k));
%         Logic = TrainDataA{Class}(:,k) > Mean;
%         Sum = length(Logic);
%         Sum1 = sum(Logic);
%         Pos1 = sum(Logic(1 : Bound));
%         Neg1 = sum(Logic(Bound+1 : end));
%         EG1 = -(Pos1/Sum1) * log(Pos1/Sum1)- Neg1/Sum1 * log(Neg1/Sum1);
%         Sum2 = Sum - Sum1;
%         Pos2 = Bound - Pos1;
%         Neg2 = Sum - Bound - Neg1;
%         EG2 = - (Pos2/Sum2) * log(Pos2/Sum2) - Neg2/Sum2 * log(Neg2/Sum2);
%         AdvancedIG(i) = ES - Sum1/Sum * EG1 - Sum2/Sum * EG2;
%         i = i + 1;
%     end
% end
% AdvancedIG_t = reshape(AdvancedIG,FeatureNum,length(AdvancedClassifier));
% [IG_SA,I_SA] = sort(AdvancedIG,'descend');
% [IG_SAF,I_SAF] = sort(mean(AdvancedIG_t,2),'descend');