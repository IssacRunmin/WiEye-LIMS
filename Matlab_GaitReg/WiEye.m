function varargout = WiEye(varargin)
% WIEYE MATLAB code for WiEye.fig
%      WIEYE, by itself, creates a new WIEYE or raises the existing
%      singleton*.
%
%      H = WIEYE returns the handle to a new WIEYE or the handle to
%      the existing singleton*.
%
%      WIEYE('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in WIEYE.M with the given input arguments.
%
%      WIEYE('Property','Value',...) creates a new WIEYE or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before WiEye_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to WiEye_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help WiEye

% Last Modified by GUIDE v2.5 26-May-2019 12:00:15

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @WiEye_OpeningFcn, ...
                   'gui_OutputFcn',  @WiEye_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before WiEye is made visible.
function WiEye_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to WiEye (see VARARGIN)

% Choose default command line output for WiEye
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes WiEye wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = WiEye_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global Running;
global AxesHandle;
ScanFileNum(handles);
AxesHandle = cell(4,1);
set(handles.CtrlButton, 'enable', 'off');
Running = false;
% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in CtrlButton.
function CtrlButton_Callback(hObject, eventdata, handles)
% hObject    handle to CtrlButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global Log;
State = get(hObject,'String');
if strcmp(State,'Start')
    set(handles.BeginButton,'enable','off');
    set(handles.DeleteButton,'enable','off');
    set(hObject,'String','End');
    Log = true;
    disp('Start logging');
else
    set(handles.BeginButton,'enable','on');
    set(handles.DeleteButton,'enable','on');
    set(hObject,'String','Start');
    Log = false;
    disp('End logging');
%     ScanFileNum(handles);
end


function FolderEdit_Callback(hObject, eventdata, handles)
% hObject    handle to FolderEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
ScanFileNum(handles);
% Hints: get(hObject,'String') returns contents of FolderEdit as text
%        str2double(get(hObject,'String')) returns contents of FolderEdit as a double


% --- Executes during object creation, after setting all properties.
function FolderEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to FolderEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in DeleteButton.
function DeleteButton_Callback(hObject, eventdata, handles)
% hObject    handle to DeleteButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global FNum;
FileDir = get(handles.FolderEdit,'String');
% FNum = ScanFileNum(handles);
FNumStr = sprintf('%02d',FNum);
if exist(fullfile(FileDir,[FNumStr '.mat']), 'file')
    delete(fullfile(FileDir,[FNumStr '.mat']));
    set(handles.CountText, 'String',...
        sprintf('%02d',max(FNum-1,0)));
    FNum = FNum - 1;
end

% --- Executes on button press in BeginButton.
function BeginButton_Callback(hObject, eventdata, handles)
% hObject    handle to BeginButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global Running;
global Log; 
global AxesHandle;
global FNum;
if (~strcmp(get(hObject, 'String'), 'Begin'))
    Running = false;
    set(hObject, 'String', 'Begin');
    set(handles.CtrlButton, 'enable', 'off');
    return;
end
set(hObject, 'String', 'Stop');
set(handles.CtrlButton,'enable','on');
FileDir = get(handles.FolderEdit, 'String');
ScanFileNum(handles);
FigH = handles.DataPanel;
for i = 1 : 4
    if ishandle(AxesHandle{i})
        delete(AxesHandle{i});
        AxesHandle{i} = [];
    end
end
Running = true;
try
    orm_data_processing_online7;
catch ME
    disp(getReport(ME));
    disp(len);
end




function Num = ScanFileNum(handles)
% FileDir: file directory
global FNum;
FileDir = get(handles.FolderEdit,'String');
FileLike = '*.mat';
if ~exist(FileDir,'dir')
    mkdir(FileDir);
end
% disp(fullfile(FileDir, FileLike))
Dirs = dir(fullfile(FileDir, FileLike));
FNum = length(Dirs);
set(handles.CountText, 'String', sprintf('%02d',FNum));
Num = FNum;