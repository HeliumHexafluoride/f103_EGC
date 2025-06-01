close all;

% 绝对路径的 CSV 文件
% filename = 'E:\my_project_new\ECGmeasuring\f103_EGC\CurveData10.csv'; % 替换为你的绝对路径
filename = 'E:\my_project_new\ECGmeasuring\f103_EGC\test2.csv'; % 替换为你的绝对路径
% 读取 CSV 文件
data = readtable(filename);

% 提取 ECG 数据（假设 ECG 数据在第二列）
ecg_signal = data{:, 3}; % 根据实际列位置修改
Fs = 250; % 采样率
% Fs = 500; % 采样率
t = (0:length(ecg_signal)-1)/Fs; % 时间轴


% 定义权重
weights = [0.1, 0.2, 0.3, 0.2, 0.1]; % 自定义权重
weights = weights / sum(weights); % 确保权重和为1

% 加权滑动平均滤波
ecg_signal = conv(ecg_signal, weights, 'same'); % 使用卷积实现加权滑动平均

% 加权滑动平均滤波
ecg_signal = conv(ecg_signal, weights, 'same'); % 使用卷积实现加权滑动平均


% 绘制时域图
figure;
subplot(2,1,1);
plot(t, ecg_signal);
xlabel('时间 (秒)');
ylabel('幅度');
title('ECG 信号时域图');
grid on;

% FFT 变换
N = length(ecg_signal); % 信号长度
Y = fft(ecg_signal); % FFT 变换
Y(1)=0;
P2 = abs(Y/N); % 双边频谱
P1 = P2(1:N/2+1); % 单边频谱
P1(2:end-1) = 2*P1(2:end-1); % 调整幅值
f = Fs*(0:(N/2))/N; % 频率轴

% 绘制频域图
subplot(2,1,2);
plot(f, 20*log10(P1));
xlabel('频率 (Hz)');
ylabel('幅度 (dB)');
title('ECG 信号频域图 (FFT)');
grid on;
xlim([0 Fs/2]); % 限制频率范围到 Nyquist 频率


% ecg_signal=filter(Hd,ecg_signal);
% ecg_signal=filter(Hd2,ecg_signal);
ecg_signal=filter(Hd3,ecg_signal);



% % 定义权重
% windowSize = 5; % 滑动窗口大小
% weights = [0.1, 0.2, 0.3, 0.2, 0.1]; % 自定义权重
% weights = weights / sum(weights); % 确保权重和为1
% 
% % 加权滑动平均滤波
% ecg_signal = conv(ecg_signal, weights, 'same'); % 使用卷积实现加权滑动平均


% FFT 变换
N = length(ecg_signal); % 信号长度
Y = fft(ecg_signal); % FFT 变换
Y(1)=0;
P2 = abs(Y/N); % 双边频谱
P1 = P2(1:N/2+1); % 单边频谱
P1(2:end-1) = 2*P1(2:end-1); % 调整幅值
f = Fs*(0:(N/2))/N; % 频率轴

figure;
subplot(2,1,1);
plot(t, ecg_signal);
xlabel('时间 (秒)');
ylabel('幅度');
title('ECG 信号时域图');
grid on;



% 绘制频域图
subplot(2,1,2);
plot(f, 20*log10(P1));
xlabel('频率 (Hz)');
ylabel('幅度 (dB)');
title('ECG 信号频域图 (FFT)');
grid on;
xlim([0 Fs/2]); % 限制频率范围到 Nyquist 频率

% 定义异常值剔除函数
function [cleaned_data] = remove_outliers(data, threshold)
    % 计算数据的均值和标准差
    mu = mean(data);
    sigma = std(data);

    % 识别异常值
    outliers = abs(data-mu) > threshold ;

    % 修正异常值
    cleaned_data = data;
    for i = 1:length(data)
        if outliers(i)
%             % 如果是异常值，用相邻点的平均值修正
%             if i == 1
%                 cleaned_data(i) = data(i+1); % 第一个点用下一个点修正
%             elseif i == length(data)
%                 cleaned_data(i) = data(i-1); % 最后一个点用前一个点修正
%             else
%                 cleaned_data(i) = (data(i-1) + data(i+1)) / 2; % 中间点用相邻两点的平均值修正
%             end
            cleaned_data(i) = threshold/2;
        end
    end
end

