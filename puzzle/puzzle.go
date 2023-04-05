package main

import (
	"context"
	"embed"
	"errors"
	"fmt"
	"github.com/golang-jwt/jwt/v5"
	"html/template"
	"log"
	"net/http"
	"time"
)

//go:embed index.html pico.css firework.js

var f embed.FS
var hmacSecret = []byte("super_secret")
var solution1 = "36"
var solution2 = "Klabauter"

func MainHandler(w http.ResponseWriter, r *http.Request) {
	tmpl, ok := r.Context().Value("tmpl").(*template.Template)
	if !ok {
		http.Error(w, "template not found", http.StatusInternalServerError)
		return
	}

	tmplData := struct {
		Stage1        bool
		Stage1Message string
		Stage2        bool
		Stage2Message string
	}{}

	claims, ok := r.Context().Value("claims").(jwt.MapClaims)
	if ok {
		s1, ok := claims["stage1"].(bool)
		if ok {
			tmplData.Stage1 = s1
		}
		s2, ok := claims["stage2"].(bool)
		if ok {
			tmplData.Stage2 = s2
		}
	}

	switch r.Method {
	case "GET":
	case "POST":
		if err := r.ParseForm(); err != nil {
			log.Printf("ParseForm() err: %v", err)
		}
		s1 := r.FormValue("solution1")
		if s1 == solution1 {
			tmplData.Stage1 = true
			tmplData.Stage1Message = ""
		} else if s1 != "" {
			tmplData.Stage1Message = "This is not the correct answer. 🙄"
		} else {
			tmplData.Stage1Message = ""
		}
		s2 := r.FormValue("solution2")
		if s2 == solution2 && tmplData.Stage1 {
			tmplData.Stage1 = true
			tmplData.Stage2 = true
			tmplData.Stage2Message = ""
		} else if s2 != "" && tmplData.Stage1 {
			tmplData.Stage2Message = "This is not the correct answer. 🙄"
		} else {
			tmplData.Stage2Message = ""
		}
		token := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
			"stage1":    tmplData.Stage1,
			"stage2":    tmplData.Stage2,
			"timestamp": time.Now().Unix(),
		})
		tokenString, err := token.SignedString(hmacSecret)
		if err != nil {
			log.Printf("SignedString() err: %v", err)
		} else {
			cookie := http.Cookie{Name: "token",
				Value:    tokenString,
				Path:     "/",
				HttpOnly: true,
				SameSite: http.SameSiteStrictMode}
			http.SetCookie(w, &cookie)
		}
	default:
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	err := tmpl.Execute(w, tmplData)
	if err != nil {
		log.Printf("Execute() err: %v", err)
	}
}

func ValidateJWT(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		cookie, err := r.Cookie("token")
		if err != nil {
			switch {
			case errors.Is(err, http.ErrNoCookie):
				log.Printf("No JWT found")
				next.ServeHTTP(w, r)
				return
			default:
				log.Println(err)
				http.Error(w, "server error", http.StatusInternalServerError)
			}
		}

		token, err := jwt.Parse(cookie.Value, func(token *jwt.Token) (interface{}, error) {
			if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, fmt.Errorf("unexpected signing method: %v", token.Header["alg"])
			}
			return hmacSecret, nil
		})

		if claims, ok := token.Claims.(jwt.MapClaims); ok && token.Valid {
			ctx := context.WithValue(r.Context(), "claims", claims)
			r = r.WithContext(ctx)
		}

		next.ServeHTTP(w, r)
	})
}

func main() {
	tmpl := template.Must(template.ParseFS(f, "index.html"))

	http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.FS(f))))

	http.HandleFunc("/reset", func(w http.ResponseWriter, r *http.Request) {
		cookie := http.Cookie{Name: "token",
			HttpOnly: true,
			MaxAge:   -1,
			SameSite: http.SameSiteStrictMode}
		http.SetCookie(w, &cookie)
		http.Redirect(w, r, "/", http.StatusSeeOther)
	})

	http.Handle("/", ValidateJWT(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		ctx := context.WithValue(r.Context(), "tmpl", tmpl)
		MainHandler(w, r.WithContext(ctx))
	})))
	http.ListenAndServe(":18080", nil)
}
