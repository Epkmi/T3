package com.bcopstein.endpointsdemo1;

import java.util.List;

public interface AcervoRepository {
List<Livro> getAll();
List<String> getTitulos();    
List<String> getAutores();    
List<Livro> getLivrosDoAutor(String autor);
Livro getLivroTitulo(Livro livro);
boolean cadastraLivroNovo(Livro livro);
boolean RemoveLivro(int Codigo);


}