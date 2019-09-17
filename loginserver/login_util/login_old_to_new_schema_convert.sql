-- Because the old / legacy schema was mostly inconsistent with naming and overall data structure, we have
-- migrated to a schema that follows our modern conventions and meanwhile fixes quite a few bugs that
-- were present as well

-- Login Accounts

INSERT INTO
  login_accounts (
    id,
    account_name,
    account_password,
    account_email,
    source_loginserver,
    last_ip_address,
    last_login_date,
    created_at
  )
SELECT
  LoginServerID,
  AccountName,
  AccountPassword,
  AccountEmail,
  'local',
  LastIPAddress,
  LastLoginDate,
  AccountCreateDate
FROM
  tblLoginServerAccounts;

-- Server Admins

INSERT INTO
  login_server_admins (
    id,
    account_name,
    account_password,
    first_name,
    last_name,
    email,
    registration_date,
    registration_ip_address
  )
SELECT
  ServerAdminID,
  AccountName,
  AccountPassword,
  FirstName,
  LastName,
  Email,
  RegistrationDate,
  RegistrationIPAddr
FROM
  tblServerAdminRegistration;

-- World Servers

INSERT INTO
  login_world_servers (
    id,
    long_name,
    short_name,
    tag_description,
    login_server_list_type_id,
    last_login_date,
    last_ip_address,
    login_server_admin_id,
    is_server_trusted,
    note
  )
SELECT
  `ServerID`,
  `ServerLongName`,
  `ServerShortName`,
  `ServerTagDescription`,
  `ServerListTypeID`,
  `ServerLastLoginDate`,
  `ServerLastIPAddr`,
  `ServerAdminID`,
  `ServerTrusted`,
  `Note`
FROM
  tblWorldServerRegistration;

DROP TABLE tblLoginServerAccounts;
DROP TABLE tblServerAdminRegistration;
DROP TABLE tblWorldServerRegistration;
DROP TABLE tblServerListType;
