#! /usr/bin/env python
#
# This script generates cross-references to show associated (handled) opcodes
# between the server and client. It will generate files for each client and
# server found, and provide some basic information..such as opcode names and
# values, server handler and whether opcodes are translated on tx/rx, etc...
#
# It's currently limited to the 'Zone' server..but, can be expounded upon to
# include other servers and clients, and other criteria and features.


import sys
import os

from time import time, ctime


DEBUG = 1  # {0 - normal, 1 - verbose, 2 - in-depth}

base_path = os.getcwd()[:-14]  # '/utils/scripts'

client_list = ['6.2', 'Titanium', 'SoF', 'SoD', 'Underfoot', 'RoF', 'RoF2', 'ClientTest']
server_list = ['Login', 'World', 'Zone', 'UCS', 'ServerTest']

client_opcodes = {}
server_opcodes = {}

client_encodes = {}
client_decodes = {}

server_handlers = {}

out_files = {}


def main():
    """ Call each method independently and track success """

    fault = False
    faults = []

    print('')

    if fault is False:
        fault = not createoutputdirectory()

        if fault is True:
            faults.append('createoutputdirectory()')

    if fault is False:
        fault = not opendebugfile()

        if fault is True:
            faults.append('opendebugfile()')

    if fault is False:
        print('Loading source data...')

        if fault is False:
            fault = not loadclientopcodes()

            if fault is True:
                faults.append('loadclientopcodes()')

        if fault is False:
            fault = not loadserveropcodes()

            if fault is True:
                faults.append('loadserveropcodes()')

        if fault is False:
            fault = not loadclienttranslators()

            if fault is True:
                faults.append('loadclienttranslators()')

        if fault is False:
            fault = not loadserverhandlers()

            if fault is True:
                faults.append('loadserverhandlers()')

        if fault is False:
            fault = not discoverserverhandlers()

            if fault is True:
                faults.append('discoverserverhandlers()')

        if fault is False:
            fault = not clearemptyserverentries()

            if fault is True:
                faults.append('clearemptyserverentries()')

    if fault is False:
        print('Creating output streams...')

        if fault is False:
            fault = not openoutputfiles()

            if fault is True:
                faults.append('openoutputfiles()')

    if fault is False:
        print('Parsing opcode data...')

        if fault is False:
            fault = not parseclientopcodedata()

            if fault is True:
                faults.append('parseclientopcodedata()')

        if fault is False:
            fault = not parseserveropcodedata()

            if fault is True:
                faults.append('parseserveropcodedata()')

    if fault is False:
        print('Destroying output streams...')

    # these should always be processed..verbose or silent
    if not closeoutputfiles():
        faults.append('closeoutputfiles()')

    if not closedebugfile():
        faults.append('closedebugfile()')

    if len(faults) > 0:
        message = 'Script failed due to errors in:\n'

        for entry in faults:
            message += '  {0}'.format(entry)

        print(message)

    return


def createoutputdirectory():
    """ Check for output directory - create if does not exist """

    try:
        output_path = '{0}/utils/scripts/opcode_handlers_output'.format(base_path)

        if DEBUG >= 1:
            print(output_path)

        if not os.path.exists(output_path):
            os.mkdir(output_path)

        return True
    except:
        if DEBUG >= 2:
            print('EXCEPTION ERROR->createoutputdirectory({0})'.format(sys.exc_info()[0]))

        return False


def opendebugfile():
    file_name = '{0}/utils/scripts/opcode_handlers_output/DEBUG.txt'.format(base_path)

    if DEBUG >= 1:
        print(file_name)

    out_files['DEBUG'] = open(file_name, 'w')

    return True


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

                for data_line in data_file:
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
        except:
            if DEBUG >= 2:
                print('EXCEPTION ERROR->loadclientopcodes({0})'.format(sys.exc_info()[0]))

            bad_clients.append(client)

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
        value = 0

        server_opcodes['OP_Unknown'] = value
        value += 1

        if DEBUG >= 2:
            print('N[Server](OP_Unknown) = {0}'.format(server_opcodes['OP_Unknown']))

        file_name = '{0}{1}'.format(
            base_path,
            '/common/emu_oplist.h')

        if DEBUG >= 1:
            print(file_name)

        with open(file_name, 'r') as data_file:
            for data_line in data_file:
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

        file_name = '{0}{1}'.format(
            base_path,
            '/common/mail_oplist.h')

        if DEBUG >= 1:
            print(file_name)

        with open(file_name, 'r') as data_file:
            for data_line in data_file:
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
    except:
        if DEBUG >= 2:
            print('EXCEPTION ERROR->loadserveropcodes({0})'.format(sys.exc_info()[0]))

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

                for data_line in data_file:
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
        except:
            if DEBUG >= 2:
                print('EXCEPTION ERROR->loadclienttranslators({0})'.format(sys.exc_info()[0]))

            return False

    # there's always going to be at least one client with one encode or decode
    if not len(client_encodes) > 0 and not len(client_decodes) > 0:
        return False

    return True


def loadserverhandlers():
    """ Load pre-designated SERVER opcode handlers """

    # TODO: handle remarked out definitions in file (i.e., // and /**/);
    bad_servers = []

    for server in server_list:
        try:
            if server == 'Login':
                if DEBUG >= 1:
                    print('No pre-designated server opcode handlers for \'{0}\''.format(server))

                continue
            elif server == 'World':
                if DEBUG >= 1:
                    print('No pre-designated server opcode handlers for \'{0}\''.format(server))

                continue
            elif server == 'Zone':
                file_name = '{0}{1}'.format(
                    base_path,
                    '/zone/client_packet.cpp')
        
                if DEBUG >= 1:
                    print(file_name)
        
                with open(file_name, 'r') as data_file:
                    server_handlers[server] = {}
                    can_run = False
                    line_no = 0

                    for data_line in data_file:
                        line_no += 1

                        if can_run is False:
                            if data_line[:19] == 'void MapOpcodes() {':
                                can_run = True

                            continue

                        if data_line[0:1] == '}':
                            break
                
                        key_begin = data_line.find('OP_')
                        key_end = data_line.find(']', key_begin)
                
                        if key_begin < 0 or key_end < 0:
                            continue
                
                        val_begin = data_line.find('Client::', key_end)
                        val_end = data_line.find(';', val_begin)
                
                        if val_begin < 0 or val_end < 0:
                            continue

                        # TODO: add continue on 'in server_opcodes' failure

                        if not data_line[key_begin:key_end] in server_handlers[server]:
                            server_handlers[server][data_line[key_begin:key_end]] = []

                        server_handlers[server][data_line[key_begin:key_end]].append(
                            '../zone/client_packet.cpp({0}:{1}) \'{2}\''.format(
                                line_no,
                                key_begin,
                                data_line[val_begin:val_end]))
                
                        if DEBUG >= 2:
                            print('[{0}][{1}]({2}) [{3}]'.format(
                                server,
                                data_line[key_begin:key_end],
                                data_line[val_begin:val_end],
                                data_line[val_begin:val_end] in server_handlers[server][data_line[key_begin:key_end]]))

                data_file.close()
            elif server == 'UCS':
                if DEBUG >= 1:
                    print('No pre-designated server opcode handlers for \'{0}\''.format(server))

                continue
            else:
                if DEBUG >= 1:
                    print('No pre-designated server opcode handlers for \'{0}\''.format(server))

                if DEBUG >= 2:
                    print('->LoadServerHandlers(Someone added a new server and forgot to code for the data load...)')

                continue
        except:
            if DEBUG >= 2:
                print('EXCEPTION ERROR->loadserverhandlers({0})'.format(sys.exc_info()[0]))

            bad_servers.append(server)

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


def discoverserverhandlers():
    """ Load undefined SERVER opcode handlers using 'discovery' method """

    locations = {}

    for server in server_list:  # initialize lists for any remaining servers
        locations[server] = []

    # manually enter search locations
    #if 'Server' in locations:
    #    locations['Server'].append('/<local_path>/<file_name>.<ext>')

    # TODO: if/how to include perl/lua handlers...

    if 'Login' in locations:
        locations['Login'].append('/loginserver/Client.cpp')
        locations['Login'].append('/loginserver/ServerManager.cpp')
        locations['Login'].append('/loginserver/WorldServer.cpp')

    if 'World' in locations:
        locations['World'].append('/world/client.cpp')

    # the bulk of opcodes are handled in 'Zone' - if processing occurs on a different
    # server, you will need to manually trace 'ServerPacket' to the deferred location
    if 'Zone' in locations:
        locations['Zone'].append('/zone/AA.cpp')
        locations['Zone'].append('/zone/attack.cpp')
        locations['Zone'].append('/zone/bot.cpp')
        locations['Zone'].append('/zone/client.cpp')
        locations['Zone'].append('/zone/client_packet.cpp')
        locations['Zone'].append('/zone/client_process.cpp')
        locations['Zone'].append('/zone/command.cpp')
        locations['Zone'].append('/zone/corpse.cpp')
        locations['Zone'].append('/zone/doors.cpp')
        locations['Zone'].append('/zone/effects.cpp')
        locations['Zone'].append('/zone/entity.cpp')
        locations['Zone'].append('/zone/exp.cpp')
        locations['Zone'].append('/zone/groups.cpp')
        locations['Zone'].append('/zone/guild.cpp')
        locations['Zone'].append('/zone/guild_mgr.cpp')
        locations['Zone'].append('/zone/horse.cpp')
        locations['Zone'].append('/zone/inventory.cpp')
        locations['Zone'].append('/zone/loottables.cpp')
        locations['Zone'].append('/zone/merc.cpp')
        locations['Zone'].append('/zone/mob.cpp')
        locations['Zone'].append('/zone/MobAI.cpp')
        locations['Zone'].append('/zone/Object.cpp')
        locations['Zone'].append('/zone/pathing.cpp')
        locations['Zone'].append('/zone/petitions.cpp')
        locations['Zone'].append('/zone/questmgr.cpp')
        locations['Zone'].append('/zone/raids.cpp')
        locations['Zone'].append('/zone/special_attacks.cpp')
        locations['Zone'].append('/zone/spells.cpp')
        locations['Zone'].append('/zone/spell_effects.cpp')
        locations['Zone'].append('/zone/tasks.cpp')
        locations['Zone'].append('/zone/titles.cpp')
        locations['Zone'].append('/zone/tradeskills.cpp')
        locations['Zone'].append('/zone/trading.cpp')
        locations['Zone'].append('/zone/trap.cpp')
        locations['Zone'].append('/zone/tribute.cpp')
        locations['Zone'].append('/zone/worldserver.cpp')
        locations['Zone'].append('/zone/zone.cpp')
        locations['Zone'].append('/zone/zonedb.cpp')
        locations['Zone'].append('/zone/zoning.cpp')

    if 'UCS' in locations:
        locations['UCS'].append('/ucs/clientlist.cpp')
        locations['UCS'].append('/ucs/database.cpp')

    for server in server_list:
        if not server in server_handlers:
            server_handlers[server] = {}

        for location in locations[server]:
            try:
                file_name = '{0}{1}'.format(
                    base_path,
                    location)

                if DEBUG >= 1:
                    print(file_name)

                with open(file_name, 'r') as data_file:
                    line_no = 0
                    hint = 'Near beginning of file'

                    for data_line in data_file:
                        line_no += 1

                        if data_line[:1].isalpha():
                            hint_end = data_line.find('(')

                            if not hint_end < 0:
                                hint_begin = hint_end - 1

                                while not hint_begin < 0:
                                    if data_line[(hint_begin - 1):hint_begin].isspace():
                                        if not data_line[hint_begin:(hint_begin + 1)].isalpha():
                                            hint_begin += 1

                                        hint = 'Near {0}'.format(data_line[hint_begin:hint_end])

                                        break

                                    hint_begin -= 1

                        op_begin = data_line.find('OP_')

                        if op_begin < 0:
                            continue

                        if data_line[(op_begin - 20):op_begin] == 'EQApplicationPacket(':
                            key_begin = op_begin
                            key_end = data_line.find(',', key_begin)
                        elif data_line[(op_begin - 12):op_begin] == '->SetOpcode(':
                            key_begin = op_begin
                            key_end = data_line.find(')', key_begin)
                        elif data_line[(op_begin - 5):op_begin] == 'case ':
                            key_begin = op_begin
                            key_end = data_line.find(':', key_begin)
                        else:
                            continue

                        if key_end < 0:
                            continue

                        if not data_line[key_begin:key_end] in server_opcodes:
                            out_files['DEBUG'].write('Illegal Opcode Found: ..{0} ({1}:{2}) \'{3}\'\n'.format(
                                location,
                                line_no,
                                key_begin,
                                data_line[key_begin:key_end]
                            ))

                            continue

                        if not data_line[key_begin:key_end] in server_handlers[server]:
                            server_handlers[server][data_line[key_begin:key_end]] = []

                        if not data_line in server_handlers[server][data_line[key_begin:key_end]]:
                            server_handlers[server][data_line[key_begin:key_end]].append(
                                '..{0}({1}:{2}) \'{3}\''.format(
                                    location,
                                    line_no,
                                    key_begin,
                                    hint))
            except:
                if DEBUG >= 2:
                    print('EXCEPTION ERROR->discoverserverhandlers({0})'.format(sys.exc_info()[0]))

    return True


def clearemptyserverentries():
    bad_servers = []

    for server in server_list:
        if len(server_handlers[server]) == 0:
            bad_servers.append(server)
        else:
            bad_opcodes = []

            for opcode in server_handlers[server]:
                if len(server_handlers[server][opcode]) == 0:
                    bad_opcodes.append(opcodes)

            for bad_opcode in bad_opcodes:
                del server_handlers[server][bad_opcode]

            if len(server_handlers[server]) == 0:
                bad_servers.append(server)

    for bad_server in bad_servers:
        if DEBUG >= 1:
            print('Deleting \'{0}\' server from search criteria...'.format(bad_server))
            print('Deleting stale entries for \'{0}\' server...'.format(bad_server))

        del server_handlers[bad_server]
        server_list.remove(bad_server)

    return True


def openoutputfiles():
    """ Open output files in 'w' mode - create/overwrite mode """

    try:
        file_name = '{0}/utils/scripts/opcode_handlers_output/Report.txt'.format(base_path)

        if DEBUG >= 1:
            print(file_name)

        out_files['Report'] = open(file_name, 'w')

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
    except:
        if DEBUG >= 2:
            print('EXCEPTION ERROR->openoutputfiles({0})'.format(sys.exc_info()[0]))

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

        if 'Report' in out_files:
            out_files['Report'].close()
            del out_files['Report']

            if DEBUG >= 2:
                print('->OpeningReportStream(exception)')

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
                encoded = 'N/A'
                decoded = 'N/A'

            message = 'Opcode: {0} ({1}) | Handled: {2} | Encoded: {3} | Decoded: {4}\n'.format(
                client_opcode,
                client_opcodes[client][client_opcode],
                handled,
                encoded,
                decoded)

            for server in server_list:
                if client_opcode in server_handlers[server] and len(server_handlers[server][client_opcode]) > 0:
                    handler_list = server_handlers[server][client_opcode]
                    handler_list.sort()

                    for handler_entry in handler_list:
                        message += '  Server: {0} ({1}) | Handler: {2}\n'.format(
                            server.ljust(len(server) + (server_max_len - len(server)), ' '),
                            '{0}'.format(server_opcodes[client_opcode]).zfill(4),
                            handler_entry)
                else:
                    message += '  Server: {0} (0000) | Handler: N/A\n'.format(
                        server.ljust(len(server) + (server_max_len - len(server)), ' '))

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
            handler_list = server_handlers[server][handler_opcode]
            handler_list.sort()

            message = ''

            for handler_entry in handler_list:
                message += 'Opcode: {0} ({1}) | Handler: {2}\n'.format(
                    handler_opcode,
                    server_opcodes[handler_opcode],
                    handler_entry)

            for client in client_list:
                if handler_opcode in client_opcodes[client]:
                    val1 = client_opcodes[client][handler_opcode]
                    val2 = 'True'
                    val3 = '{0}'.format(handler_opcode in client_encodes[client])
                    val4 = '{0}'.format(handler_opcode in client_decodes[client])
                else:
                    val1 = '0x0000'
                    val2 = 'False'
                    val3 = 'N/A'
                    val4 = 'N/A'

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

    if 'Report' in out_files:
        out_files['Report'].close()
        del out_files['Report']

        if DEBUG >= 2:
            print('->ClosingReportStream')

    return True


def closedebugfile():
    if 'DEBUG' in out_files:
        out_files['DEBUG'].close()
        del out_files['DEBUG']

        if DEBUG >= 2:
            print('->ClosingDEBUGStream')

    return True


if __name__ == '__main__':
    main()
