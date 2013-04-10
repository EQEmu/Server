#! /usr/bin/env python
# Requires MySQLdb (python-mysqldb)

import MySQLdb as mdb
import xml.etree.ElementTree as ET
import sys

def main():
    con = None
    table = "tradeskill_recipe_entries"
    db = "eq"
    user = "eq"
    password = "eq"
    host = "localhost"

    try:
        tree = ET.parse('eqemu_config.xml')
        database = tree.getroot().find('database')
        db = database.find('db').text
        user = database.find('username').text
        password = database.find('password').text
        host = database.find('host').text

    except:
        print("Trying with defaults")
        print("Do you have a valid XML?")

    try:
        con = mdb.connect(host, user, password, db)

        cur = con.cursor()
        cur.execute("SELECT MAX(id) FROM `tradeskill_recipe")
        data = cur.fetchone()
        highest = data[0]
        for recipe_id in range(1, highest+1):
            complist = []
            cur.execute("SELECT item_id,componentcount FROM " + table + " WHERE componentcount>0 AND recipe_id=" + str(recipe_id))
            rows = cur.fetchall()
            for row in rows:
                complist.append(row)

            faillist = []
            cur.execute("SELECT item_id,failcount FROM " + table + " WHERE failcount>0 AND recipe_id=" + str(recipe_id))
            rows = cur.fetchall()
            for row in rows:
                faillist.append(row)

            # Remove any items that are returned on a failure
            for item in faillist:
                if item in complist:
                    complist.remove(item)

            # Remove some items that don't make sense
            for item in faillist:
                if item[0] == 10062:
                    for item2 in complist:
                        if item2[0] == 93510:
                            complist.remove(item2)
                if item[0] == 93618:
                    for item2 in complist:
                        if item2[0] in [93508, 93509]:
                            complist.remove(item2)


            for item in complist:
                cur.execute("UPDATE `" + table + "` SET `salvagecount` = '" + str(item[1]) + "' WHERE `componentcount` > '0' AND `item_id` = '" + str(item[0]) + "' AND `recipe_id` = '" + str(recipe_id) + "';")

    except mdb.Error, e:
        print("Error %d: %s", e.args[0],e.args[1])
        sys.exit(1)

    finally:
        if con:
            con.close()

    return 0

if __name__ == '__main__':
    main()
