alter table books add language int not null default 0;

drop table if exists reading_is_fundamental;

create table reading_is_fundamental
(
filename varchar(32),
language int
);

insert into reading_is_fundamental (select items.filename, items.booktype from items where items.filename  != "" group by filename);

update books set books.language = (select language from reading_is_fundamental r where r.filename = books.name);

drop table reading_is_fundamental;
