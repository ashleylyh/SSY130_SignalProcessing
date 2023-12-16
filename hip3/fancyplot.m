classdef fancyplot
    properties
    end
    
    methods
%         function obj = fancyplot(inputArg1,inputArg2)
%             %FANCYPLOT Construct an instance of this class
%             %   Detailed explanation goes here
%             obj.Property1 = inputArg1 + inputArg2;
%         end
    end
    
    methods(Static)
        
        % varargin{1}: figure handle to be saved. If not specified -> gcf
        function savefig(fname,varargin)
            figformat = 'epsc';
            folder = fullfile(pwd,'images');
            if nargin <=1
                fighandle = gcf;
            else
                fighandle = varargin{1};
            end
            if ~ exist(folder,'dir'), mkdir(folder); end
            set(gca,'LooseInset',get(gca,'TightInset'))
            saveas(fighandle, fullfile(folder,fname),figformat)
        end
        
        function latexcode = m2latex(matrix)
            if ~isa(matrix,'sym')
                matrix = sym(matrix);
            end
            latexcode = latex(matrix)
            clipboard('copy',latexcode);
        end
        
        % varargin{1}: line opacity
        function cl = getColor(n, varargin)
            colrs = lines(20);
            if nargin >= 2
                opacity = varargin{1};
            end
            cl = [colrs(n,:), opacity];
        end
%         function fighandle = plot(obj,inputArg)
%             figure('Color','white','Position',[464.2000e+000   373.8000e+000   404.0000e+000   290.4000e+000]);
%             xlabel 'time [s]', ylabel 'velocity [km/h]', hold on, grid on
%             % axis([0 600 0 220])
%             plot(t_euler,vel.observation,'--','Color',lc(2,:),'LineWidth',1);
%             plot(t_euler,vel.signal,'Color',lc(1,:),'LineWidth',2);
%             legend({'Euler - measured','Euler - true'});   
%         end
    end
end
