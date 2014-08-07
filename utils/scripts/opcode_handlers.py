#! /usr/bin/env python
#
# This script generates cross-references to show associated (handled) opcodes
# between the server and client. It will generate files for each client and
# server found, and provide some basic information..such as opcode names and
# values, server handler and whether opcodes are translated on tx/rx, etc...
#
# It's currently limited to the 'Zone' server..but, can be expounded upon to
# include other servers and clients, and other criteria and features.


import os
# import pickle

from time import time, ctime


DEBUG = 1  # {0 - normal, 1 - verbose, 2 - in-depth}

base_path = os.getcwd()[:-14]  # '/utils/scripts'

client_list = ['6.2', 'Titanium', 'SoF', 'SoD', 'Underfoot', 'RoF']
server_list = ['Login', 'World', 'Zone', 'UCS']

client_opcodes = {}
server_opcodes = {}

client_encodes = {}
client_decodes = {}

server_handlers = {}

out_files = {}


def main():
    print('Loading source data...')

    if not loadclientopcodes():
        print('Could not load client opcodes...')
        return

    if not loadserveropcodes():
        print('Could not load server opcodes...')
        return

    if not loadclienttranslators():
        print('Could not load client translators...')
        return
    
    if not loadserverhandlers():
        print('Could not load server handlers...')
        return

    print('Creating output streams...')

    if not createoutputdirectory():
        print('Could not create output directory...')
        return
        
    if not openoutputfiles():
        print('Could not open output files...')
        return

    print('Parsing opcode data...')

    if not parseclientopcodedata():
        print('Could not parse client opcode data...')
        return

    if not parseserveropcodedata():
        print('Could not parse server opcode data...')
        return

    print('Destroying output streams...')

    if not closeoutputfiles():
        print('Could not close output files...')
        return


def loadclientopcodes():
    bad_clients = []
    
    for client in client_list:
        try:
            short_name = '{0}{1}{2}'.format(
                '/patch_',
                client,
                '.conf')

            file_name = '{0}{1}{2}'.format(
                base_path,
                '/utils/patches',
                short_name)
            
            if DEBUG >= 1:
                print(file_name)

            with open(file_name, 'r') as data_file:
                client_opcodes[client] = {}  # force empty dictionary to avoid collisions

                while True:
                    data_line = data_file.readline()

                    if not data_line:
                        break

                    key_begin = data_line.find('OP_')
                    key_end = data_line.find('=', key_begin)

                    if not key_begin == 0 or key_end < 0:
                        continue

                    val_begin = data_line.find('0x', key_end)
                    val_end = val_begin + 6  # max size is always 6 bytes

                    if val_begin < 0:
                        continue

                    value = int(data_line[(val_begin + 2):val_end].lower(), 16)

                    if value == 0:
                        continue

                    client_opcodes[client][data_line[key_begin:key_end]] = '0x{0}'.format(hex(value)[2:].zfill(4))

                    if DEBUG >= 2:
                        print('[{0}][{1}] = {2} (int: {3})'.format(
                            client,
                            data_line[key_begin:key_end],
                            client_opcodes[client][data_line[key_begin:key_end]],
                            value))

            data_file.close()
        except:  # StandardError as se:
            #print(se.message)

            #if DEBUG >= 2:
            #    print(pickle.dumps(se.args))

            bad_clients.append(client)

    if len(bad_clients) > 0:
        for bad_client in bad_clients:
            if DEBUG >= 1:
                print('Deleting \'{0}\' client from search criteria...'.format(bad_client))
            
            client_list.remove(bad_client)

            if DEBUG >= 1:
                print('Deleting stale entries for \'{0}\' client...'.format(bad_client))

            if bad_client in client_opcodes:
                del client_opcodes[bad_client]
                
    if not len(client_list) > 0:
        return False

    return True


def loadserveropcodes():
    try:
        value = 1  # Server opcodes are auto-enumerated with a starting reference of '1'

        file_name = '{0}{1}{2}'.format(
            base_path,
            '/common',
            '/emu_oplist.h')

        if DEBUG >= 1:
            print(file_name)

        with open(file_name, 'r') as data_file:
            while True:
                data_line = data_file.readline()

                if not data_line:
                    break

                val_begin = data_line.find('OP_', 2)
                val_end = data_line.find(')', val_begin)

                if val_begin < 0 or val_end < 0:
                    continue

                if data_line[:1] == 'N':
                    server_opcodes[data_line[val_begin:val_end]] = value
                    value += 1

                    if DEBUG >= 2:
                        print('N[{0}]({1}) = {2}'.format(
                            'Server',
                            data_line[val_begin:val_end],
                            server_opcodes[data_line[val_begin:val_end]]))

        data_file.close()

        file_name = '{0}{1}{2}'.format(
            base_path,
            '/common',
            '/mail_oplist.h')

        if DEBUG >= 1:
            print(file_name)

        with open(file_name, 'r') as data_file:
            while True:
                data_line = data_file.readline()

                if not data_line:
                    break

                val_begin = data_line.find('OP_', 2)
                val_end = data_line.find(')', val_begin)

                if val_begin < 0 or val_end < 0:
                    continue

                if data_line[:1] == 'N':
                    server_opcodes[data_line[val_begin:val_end]] = value
                    value += 1

                    if DEBUG >= 2:
                        print('N[{0}]({1}) = {2}'.format(
                            'Server',
                            data_line[val_begin:val_end],
                            server_opcodes[data_line[val_begin:val_end]]))

        data_file.close()
    except:  # StandardError as se:
        #print(se.message)

        #if DEBUG >= 2:
        #    print(pickle.dumps(se.args))

        return False

    return True


def loadclienttranslators():
    for client in client_list:
        if client == '6.2':
            short_name = '{0}'.format('/Client62_ops.h')
        else:
            short_name = '{0}{1}{2}'.format(
                '/',
                client,
                '_ops.h')
        
        try:
            file_name = '{0}{1}{2}'.format(
                base_path,
                '/common/patches',
                short_name)
            
            if DEBUG >= 1:
                print(file_name)

            with open(file_name, 'r') as data_file:
                client_encodes[client] = []
                client_decodes[client] = []

                while True:
                    data_line = data_file.readline()
                    
                    if not data_line:
                        break

                    val_begin = data_line.find('OP_', 2)
                    val_end = data_line.find(')', val_begin)

                    if val_begin < 0 or val_end < 0:
                        continue
                    
                    if data_line[:1] == 'E':
                        client_encodes[client].append(data_line[val_begin:val_end])
                        
                        if DEBUG >= 2:
                            print('E[{0}]({1}) (listed: {2})'.format(
                                client,
                                data_line[val_begin:val_end],
                                data_line[val_begin:val_end] in client_encodes[client]))
                    elif data_line[:1] == 'D':
                        client_decodes[client].append(data_line[val_begin:val_end])
                        
                        if DEBUG >= 2:
                            print('D[{0}]({1}) (listed: {2})'.format(
                                client,
                                data_line[val_begin:val_end],
                                data_line[val_begin:val_end] in client_decodes[client]))

            data_file.close()
        except:  # StandardError as se:
            #print(se.message)

            #if DEBUG >= 2:
            #    print(pickle.dumps(se.args))

            return False

    # there's always going to be at least one client with one encode or decode
    if not len(client_encodes) > 0 and not len(client_decodes) > 0:
        return False

    return True


def loadserverhandlers():
    # TODO: handle remarked out definitions in file (i.e., // and /**/);
    # TODO: out-going-only handlers need to be added..more research...
    bad_servers = []

    for server in server_list:
        try:
            if server == 'Login':
                raise
            elif server == 'World':
                raise
            elif server == 'Zone':
                file_name = '{0}{1}{2}'.format(
                    base_path,
                    '/zone',
                    '/client_packet.cpp')
        
                if DEBUG >= 1:
                    print(file_name)
        
                with open(file_name, 'r') as data_file:
                    server_handlers[server] = {}

                    data_line = data_file.readline()
            
                    while not data_line[:19] == 'void MapOpcodes() {':
                        data_line = data_file.readline()
                
                        if not data_line:
                            break
            
                    while True:
                        data_line = data_file.readline()
                
                        if not data_line or data_line[0:1] == '}':
                            break
                
                        key_begin = data_line.find('OP_')
                        key_end = data_line.find(']', key_begin)
                
                        if key_begin < 0 or key_end < 0:
                            continue
                
                        val_begin = data_line.find('Client::', key_end)
                        val_end = data_line.find(';', val_begin)
                
                        if val_begin < 0 or val_end < 0:
                            continue

                        server_handlers[server][data_line[key_begin:key_end]] = data_line[val_begin:val_end]
                
                        if DEBUG >= 2:
                            print('[{0}][{1}] = {2}'.format(
                                server,
                                data_line[key_begin:key_end],
                                server_handlers[server][data_line[key_begin:key_end]]))

                data_file.close()
            elif server == 'UCS':
                raise
            else:
                if DEBUG >= 2:
                    print('->LoadServerHandlers(Someone added a new server and forgot to code for the data load...)')

                return False
        except:  # StandardError as se:
            #print(se.message)

            #if DEBUG >= 2:
            #    print(pickle.dumps(se.args))

            bad_servers.append(server)

    if len(bad_servers) > 0:
        for bad_server in bad_servers:
            if DEBUG >= 1:
                print('Deleting \'{0}\' server from search criteria...'.format(bad_server))

            server_list.remove(bad_server)

            if DEBUG >= 1:
                print('Deleting stale entries for \'{0}\' server...'.format(bad_server))

            if bad_server in server_handlers:
                del server_handlers[bad_server]

    if not len(server_list) > 0:
        return False

    return True


def createoutputdirectory():
    try:
        output_path = '{0}{1}'.format(
            base_path,
            '/utils/scripts/opcode_handlers_output')
        
        if DEBUG >= 1:
            print(output_path)
            
        if not os.path.exists(output_path):
            os.mkdir(output_path)
        
        return True
    except:  # StandardError as se:
        #print(se.message)

        #if DEBUG >= 2:
        #    print(pickle.dumps(se.args))

        return False


def openoutputfiles():
    try:
        #file_name = '{0}{1}{2}'.format(
        #    base_path,
        #    '/utils/scripts/opcode_handlers_output/',
        #    'Report.txt')

        #if DEBUG >= 1:
        #    print(file_name)

        #out_files['Report'] = open(file_name, 'w')

        for client in client_list:
            file_name = '{0}{1}{2}{3}'.format(
                base_path,
                '/utils/scripts/opcode_handlers_output/',
                client,
                '_opcode_handlers.txt')
            
            if DEBUG >= 1:
                print(file_name)
            
            out_files[client] = open(file_name, 'w')

            message = \
                '>> \'Opcode-Handler\' analysis for \'{0}\' client\n' \
                '>> file generated @ {1}\n' \
                '\n'.format(
                    client,
                    ctime(time()))

            out_files[client].write(message)

            if DEBUG >= 2:
                print(message[:-2])

        for server in server_list:
            file_name = '{0}{1}{2}{3}'.format(
                base_path,
                '/utils/scripts/opcode_handlers_output/',
                server,
                '_opcode_handlers.txt')

            if DEBUG >= 1:
                print(file_name)

            out_files[server] = open(file_name, 'w')

            message = \
                '>> \'Opcode-Handler\' analysis for \'{0}\' server\n' \
                '>> file generated @ {1}\n' \
                '\n'.format(
                    server,
                    ctime(time()))

            out_files[server].write(message)

            if DEBUG >= 2:
                print(message[:-2])
    except:  # StandardError as se:
        #print(se.message)

        #if DEBUG >= 2:
        #    print(pickle.dumps(se.args))

        for client in client_list:
            if client in out_files:
                out_files[client].close()
                del out_files[client]

                if DEBUG >= 2:
                    print('->OpeningClientStream(exception): {0}'.format(client))
        
        for server in server_list:
            if server in out_files:
                out_files[server].close()
                del out_files[server]

                if DEBUG >= 2:
                    print('->OpeningServerStream(exception): {0}'.format(server))

        #if 'Report' in out_files:
        #    out_files['Report'].close()
        #    del out_files['Report']

        #    if DEBUG >= 2:
        #        print('->OpeningReportStream(exception)')

        return False

    return True


def parseclientopcodedata():
    # TODO: add metrics
    for client in client_list:
        server_max_len = 0

        for server in server_list:
            if len(server) > server_max_len:
                server_max_len = len(server)

        client_keys = client_opcodes[client].keys()
        client_keys.sort()

        for client_opcode in client_keys:
            handled = client_opcode in server_opcodes

            if handled is True:
                encoded = client_opcode in client_encodes[client]
                decoded = client_opcode in client_decodes[client]
            else:
                encoded = 'n/a'
                decoded = 'n/a'

            message = 'Opcode: {0} ({1}) | Handled: {2} | Encoded: {3} | Decoded: {4}\n'.format(
                client_opcode,
                client_opcodes[client][client_opcode],
                handled,
                encoded,
                decoded)

            for server in server_list:
                if client_opcode in server_handlers[server]:
                    val1 = '{0}'.format(server_opcodes[client_opcode]).zfill(4)
                    val2 = server_handlers[server][client_opcode]
                else:
                    val1 = '0000'
                    val2 = 'n/a'

                message += '  Server: {0} ({1}) | Handler: \'{2}\'\n'.format(
                    server.ljust(len(server) + (server_max_len - len(server)), ' '),
                    val1,
                    val2)

                if DEBUG >= 2:
                    print('->EndOfServerLoop: {0}'.format(server))

            message += '\n'

            out_files[client].write(message)

            if DEBUG >= 2:
                print(message[:-2])
                print('->EndOfOpcodeLoop: {0}'.format(client_opcode))

        if DEBUG >= 2:
            print('->EndOfClientLoop: {0}'.format(client))

    return True


def parseserveropcodedata():
    # TODO: add metrics
    for server in server_list:
        client_max_len = 0

        for client in client_list:
            if len(client) > client_max_len:
                client_max_len = len(client)

        handler_keys = server_handlers[server].keys()
        handler_keys.sort()

        for handler_opcode in handler_keys:
            message = 'Opcode: {0} ({1}) | Handler: \'{2}\'\n'.format(
                handler_opcode,
                server_opcodes[handler_opcode],
                server_handlers[server][handler_opcode])

            for client in client_list:
                if handler_opcode in client_opcodes[client]:
                    val1 = client_opcodes[client][handler_opcode]
                    val2 = 'True'
                    val3 = '{0}'.format(handler_opcode in client_encodes[client])
                    val4 = '{0}'.format(handler_opcode in client_decodes[client])
                else:
                    val1 = '0x0000'
                    val2 = 'False'
                    val3 = 'n/a'
                    val4 = 'n/a'

                message += '  Client: {0} ({1}) | Handled: {2} | Encoded: {3} | Decoded: {4}\n'.format(
                    client.ljust(len(client) + (client_max_len - len(client)), ' '),
                    val1,
                    val2.ljust(len(val2) + (len('False') - len(val2)), ' '),
                    val3.ljust(len(val3) + (len('False') - len(val3)), ' '),
                    val4.ljust(len(val4) + (len('False') - len(val4)), ' '))

                if DEBUG >= 2:
                    print('->EndOfClientLoop: {0}'.format(client))

            message += '\n'

            out_files[server].write(message)

            if DEBUG >= 2:
                print(message[:-2])
                print('->EndOfOpcodeLoop: {0}'.format(handler_opcode))

        if DEBUG >= 2:
            print('->EndOfServerLoop: {0}'.format(server))

    return True


def closeoutputfiles():
    for client in client_list:
        if client in out_files:
            out_files[client].close()
            del out_files[client]

            if DEBUG >= 2:
                print('->ClosingClientStream: {0}'.format(client))

    for server in server_list:
        if server in out_files:
            out_files[server].close()
            del out_files[server]

            if DEBUG >= 2:
                print('->ClosingServerStream: {0}'.format(server))

    #if 'Report' in out_files:
    #    out_files['Report'].close()
    #    del out_files['Report']

    #    if DEBUG >= 2:
    #        print('->ClosingReportStream')

    return True


if __name__ == '__main__':
    main()
