package com.bcopstein.endpointsdemo1;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.stream.Collectors;

@Service
public class ListaService {

    private final Acervo acervo;

    @Autowired
    public ListaService(Acervo acervo) {
        this.acervo = acervo;
    }

    public long contarObrasDoAutor(String autor) {
        return acervo.getAll().stream()
                .filter(livro -> livro.autor().equals(autor))
                .count();
    }

    public long contarObrasMaisRecentesQue(int ano) {
        return acervo.getAll().stream()
                .filter(livro -> livro.ano() > ano)
                .count();
    }

    public double calcularNumeroMedioObrasPorAutor() {
        List<String> autores = acervo.getAll().stream()
                .map(Livro::autor)
                .distinct()
                .collect(Collectors.toList());

        if (autores.isEmpty()) {
            return 0;
        }

        long totalObras = acervo.getAll().size();
        return (double) totalObras / autores.size();
    }
}
