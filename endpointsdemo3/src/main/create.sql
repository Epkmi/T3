DROP TABLE livros IF EXISTS;
CREATE TABLE livros (
    codigo long AUTO_INCREMENT PRIMARY KEY,
    titulo VARCHAR(255),
    autor VARCHAR(255) ,
    ano_publicacao INT
);