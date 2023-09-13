package com.bcopstein.endpointsdemo1;

import java.util.LinkedList;
import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestBody;

@RestController
@RequestMapping("/biblioteca")
public class DemoController{
    private Acervo acervo;

    @Autowired
    public DemoController(Acervo acervo){
       this.acervo = acervo;
    }
    @GetMapping("/")
    public String getSaudacao() {
        return "Bem vindo as biblioteca central!";}   

      @GetMapping("/livros")
    public List<Livro> getLivros(){
        return acervo.getAll();}   

         @GetMapping("/titulos")
    public List<String> getTitulos(){
        return acervo.getTitulos();}   
        
        @GetMapping("/autores")
    public List<String> getAutores(){
        return acervo.getAutores();}   

    @GetMapping("/livrosautor")
    public List<Livro> getLivrosdoautor(@RequestParam(value = "autor") String autor){
        return acervo.getLivrosDoAutor(autor);}   

        @GetMapping("/livrotitulo/{titulo}")
        public ResponseEntity<Livro> getLivroTitulo(@PathVariable("titulo") String titulo) {
            return acervo.getLivroTitulo(titulo);
        }

        @PostMapping("/novolivro")
        public boolean cadastraLivroNovo(@RequestBody final Livro livro) {
            return acervo.cadastraLivroNovo(livro);
        }
    }
