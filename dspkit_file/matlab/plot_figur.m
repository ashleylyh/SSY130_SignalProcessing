close all;
clc;
clear all;
if ~ exist(fullfile(pwd,'images'),'dir'), mkdir images; end


%% Question 2

f_s = 16000;    % Hz
% h=hchannel()
h_reversedOrder_BB = [-.00215686;
        -.000338629;
        .00120097;
        .00233086;
        .00270262;
        .00289784;
        .0028333;
        .00286812;
        .00273787;
        .00221602;
        .00166554;
        .000905706;
        .000227661;
        -4.6597e-5;
        -.000596544;
        -.000568773;
        -.000110951;
        -9.91957e-5;
        -.000187978;
        -.000736488;
        -.00217947;
        -.00372665;
        -.00492334;
        -.00610065;
        -.00660802;
        -.00641812;
        -.0061574;
        -.00548766;
        -.00503206;
        -.00540669;
        -.00555536;
        -.00608644;
        -.00666502;
        -.00604582;
        -.00505088;
        -.00338963;
        -.000678661;
        .000914293;
        .00161294;
        .00264155;
        .00400973;
        .00710533;
        .00997449;
        .0110865;
        .011959;
        .013499;
        .01437;
        .0129008;
        .010227;
        .00788016;
        .00873017;
        .0123433;
        .0135395;
        .0121574;
        .00966037;
        .00691991;
        .00526697;
        .0012025;
        -.00553806;
        -.0105394;
        -.0164988;
        -.0224755;
        -.0247927;
        -.0247292;
        -.0204532;
        -.0150302;
        -.0129432;
        -.0103486;
        -.00358191;
        .00752905;
        .0183431;
        .0204146;
        .0135793;
        .00485778;
        -.000937852;
        -.00210526;
        -.000492387;
        .000251571;
        -.00014849;
        -.000428738;
        -.000433988;
        -.000343138;
        -.000346684;
        -.000333834;
        -.000340739;
        -.000324358;
        -.000327119;
        -.000277425;
        -.0003297;
        -.000296873;
        -.000259911;
        -.00021805;
        -.000185247;
        -.000235213;
        -.000177844;
        -.000171518;
        -2.14137e-5;
        2.94701e-6;
        -5.91838e-5;
        -.000181814;
        -.000177899;
        -.000140619;
        -.000127249;
        -.000103027;
        -.00018298;
        -.000306957;
        -.000287806;
        -.000200421;
        -9.53321e-5;
        3.33885e-5;
        -2.61636e-5;
        -5.69531e-5;
        -.000151486;
        -.000284678;
        -.000231088;
        -.000322049;
        -.000361783;
        -.000372629;
        -.000414153;
        -.000349757;
        -.000402473;
        -.000182193;
        -.000214891;
        -.000218312;
        -.000136544;
        -.000153254;
        -1.42141e-5;
        3.05342e-5];

h_reversedOrder_sin = [-.000101825;
        -.0002706;
        -.000432483;
        -.000581641;
        -.000713804;
        -.000825703;
        -.000912136;
        -.000972716;
        -.00100451;
        -.00100674;
        -.000979151;
        -.000920929;
        -.000835916;
        -.000724274;
        -.000590941;
        -.000438835;
        -.000273559;
        -9.76636e-5;
        8.05606e-5;
        .000259738;
        .000430338;
        .000589588;
        .000732544;
        .00085456;
        .000951005;
        .00101963;
        .00105862;
        .00106597;
        .00104047;
        .000984192;
        .000897933;
        .000785204;
        .000648717;
        .000491881;
        .000319937;
        .000138015;
        -4.93673e-5;
        -.0002353;
        -.00041695;
        -.000585361;
        -.000737611;
        -.000868271;
        -.000973475;
        -.00105001;
        -.00109649;
        -.0011096;
        -.00109057;
        -.00103841;
        -.000955804;
        -.000843709;
        -.000705881;
        -.000547106;
        -.000370057;
        -.000182229;
        1.34222e-5;
        .000209881;
        .000400839;
        .000581614;
        .000745731;
        .000888587;
        .00100538;
        .00109278;
        .00114812;
        .00116823;
        .00115412;
        .00110418;
        .00102185;
        .000907855;
        .000766643;
        .000602097;
        .000418388;
        .000222384;
        1.81834e-5;
        -.000186659;
        -.000387597;
        -.000577659;
        -.000752125;
        -.000904216;
        -.00103121;
        -.00112788;
        -.00119231;
        -.00122032;
        -.00121277;
        -.0011686;
        -.00108941;
        -.000977115;
        -.000834919;
        -.000666486;
        -.000476483;
        -.000272371;
        -5.80151e-5;
        .000158565;
        .000370528;
        .000574109;
        .000760168;
        .000925002;
        .00106107;
        .00116777;
        .00123837;
        .00127364;
        .00127067;
        .00122952;
        .00115189;
        .00103879;
        .000894896;
        .00072291;
        .000527563;
        .000316037;
        9.30947e-5;
        -.000132429;
        -.000355356;
        -.000568475;
        -.000765995;
        -.000941122;
        -.00108877;
        -.00120382;
        -.0012832;
        -.00132471;
        -.00132725;
        -.00129008;
        -.00121436;
        -.00110203;
        -.000955938;
        -.000780237;
        -.000581045;
        -.000361686;
        -.000131003;
        .000105105];




h = flipud(h_reversedOrder_sin);
[H,w] = freqz(h,1,512,f_s); %'whole',


figure('Color','white')
stem(0:numel(h)-1, h); grid on;
xlabel 'Samples', ylabel 'hsin(n) - Filter coefficients', title 'Estimated Filter hsin(n)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','h-coeffs'),'epsc')

figure('Color','white');
subplot(2,1,1);
plot(2*w,(abs(H))); grid on;
xlabel 'Frequency', ylabel 'Magnitude', title 'Estimated Filter Hsin(w)'
subplot(2,1,2)
plot(2*w,(angle(H))); grid on;
xlabel 'Frequency ', ylabel 'Phase (degrees)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','Hsin'),'epsc')





%% Question 3

f_s = 16000;    % Hz
h = [1 0 0 0.5];
[H,w] = freqz(h,1,512,f_s); %'whole',

figure('Color','white');
subplot(2,1,1);
plot(w/f_s,20*log10(abs(H))); grid on;
xlabel 'Normalized Frequency (\times \pi [rad/sample])', ylabel 'Magnitude (dB)', title 'Frequency response H(w) of h = [1, 0, 0, 0.5]'
subplot(2,1,2)
plot(w/f_s, 180/pi*wrapToPi(phase(H))); grid on;
xlabel 'Normalized Frequency (\times \pi [rad/sample])', ylabel 'Phase (degrees)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','H-echo'),'epsc')


%% Question 6


f_s = 16000;    % Hz

h = flipud(h_BB_reversedOrder_max);
figure('Color','white')
stem(0:numel(h)-1, h); grid on;
xlabel 'Samples', ylabel 'hBB(n) - Filter coefficients', title 'Estimated Filter hBB(n)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','h-coeffs-SIN-MAX'),'epsc')

h = flipud(h_reversedOrder_100);
figure('Color','white')
stem(0:numel(h)-1, h); grid on;
xlabel 'Samples', ylabel 'hBB(n) - Filter coefficients', title 'Estimated Filter hBB(n)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','h-coeffs-SIN-100'),'epsc')

h = flipud(h_reversedOrder_10);
figure('Color','white')
stem(0:numel(h)-1, h); grid on;
xlabel 'Samples', ylabel 'h(n) - Filter coefficients', title 'Estimated Filter h(n)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','h-coeffs-SIN-10'),'epsc')

[H,w] = freqz(h,1,512,f_s); %'whole',
figure('Color','white');
subplot(2,1,1);
plot(w/f_s,20*log10(abs(H))); grid on;
xlabel 'Normalized Frequency (\times \pi [rad/sample])', ylabel 'Magnitude (dB)', title 'Estimated Filter H(w)'
subplot(2,1,2)
plot(w/f_s, 180/pi*wrapToPi(angle(H))); grid on;
xlabel 'Normalized Frequency (\times \pi [rad/sample])', ylabel 'Phase (degrees)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','H-SIN-MAX'),'epsc')

%%%%%%%%%%%%

h = flipud(h_BB_reversedOrder_max);
figure('Color','white')
stem(0:numel(h)-1, h); grid on;
xlabel 'Samples', ylabel 'h(n) - Filter coefficients', title 'Estimated Filter h(n)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','h-coeffs-SIN-MAX-inc'),'epsc')

h = flipud(h_reversedOrder_100_inc);
figure('Color','white')
stem(0:numel(h)-1, h); grid on;
xlabel 'Samples', ylabel 'h(n) - Filter coefficients', title 'Estimated Filter h(n)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','h-coeffs-SIN-100-inc'),'epsc')

h = flipud(h_reversedOrder_10_inc);
figure('Color','white')
stem(0:numel(h)-1, h); grid on;
xlabel 'Samples', ylabel 'h(n) - Filter coefficients', title 'Estimated Filter h(n)'
set(gca,'LooseInset',get(gca,'TightInset'))
saveas(gcf, fullfile(pwd,'images/','h-coeffs-SIN-10-inc'),'epsc')
