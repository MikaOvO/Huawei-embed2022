import os
from unittest import result

data_dir = os.getcwd() + '\\data'
output_dir = os.getcwd() + '\\output'
main_file = os.getcwd() + '\\player_page\\code\\main.cpp'

os.system('g++ -o main.exe %s' % (main_file))

for path, dir_list, file_list in os.walk(data_dir):
    for file in file_list:
        ## only run sample
        ##if file != '0.in':
        ##    continue
        data_file = os.path.join(path, file)
        solution_dir = output_dir + '\\' + file
        if not os.path.exists(solution_dir):
            os.makedirs(solution_dir)
        ## modify here
        
        result_file = output_dir + '\\' +  'results.txt'
        debug_file = solution_dir + '\\' + 'debug.txt'
        output_file = solution_dir + '\\' + 'output.txt'

        info = 'x!' ## 控制是否打印到result里,debug的时候设置为! 否则设置为任意其他字符
        
        params_map = {}
        params_map['info'] = info
        params_map['data_file'] = data_file
        params_map['debug_file'] = debug_file
        params_map['output_file'] = output_file
        params_map['result_file'] = result_file
        
        params = ''
        for k in params_map:
            params += ' ' + k + '=' + params_map[k]
        
        ##
        
        os.system('main.exe%s' % (params))