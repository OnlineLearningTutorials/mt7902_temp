# MT7902 드라이버 가이드 기여 방법 (Contribution Guide)

작성한 가이드를 원본 저장소(`OnlineLearningTutorials/mt7902_temp`)에 반영하여 다른 사용자들이 도움을 받을 수 있도록 기여하는 방법은 크게 두 가지가 있습니다.

---

## 방법 1: Pull Request (권장 - 가장 전문적인 방법)

GitHub의 표준 기여 방식입니다. 본인의 계정으로 저장소를 복사(Fork)한 뒤, 수정사항을 제안하는 방식입니다.

### 1단계: GitHub에서 Fork 하기
1. [OnlineLearningTutorials/mt7902_temp](https://github.com/OnlineLearningTutorials/mt7902_temp) 페이지에 접속합니다.
2. 우측 상단의 **[Fork]** 버튼을 클릭하여 본인의 계정으로 저장소를 복사합니다.

### 2단계: 본인의 저장소를 연결하고 파일 업로드 (터미널 이용)
이미 로컬에 폴더가 있으므로, 원격 저장소 주소만 본인의 것으로 추가하면 됩니다.

```bash
# 본인의 Fork 저장소 주소를 추가 (username 부분에 본인 GitHub ID 입력)
git remote add my-fork https://github.com/본인의ID/mt7902_temp.git

# 새로운 브랜치 생성 (가이드 추가용)
git checkout -b add-docs-korean

# 변경사항 반영
git add docs/MT7902_installation_guide.md
git commit -m "Add Korean installation guide for MT7902"

# 본인의 저장소로 푸시
git push my-fork add-docs-korean
```

### 3단계: Pull Request 생성
1. 푸시를 완료하면 본인의 GitHub 저장소 페이지에 **[Compare & pull request]** 버튼이 나타납니다.
2. 해당 버튼을 클릭하고, 가이드의 내용을 설명한 뒤 **[Create pull request]**를 누르면 끝납니다.

---

## 방법 2: Issue 작성 (가장 쉬운 방법)

Git 명령어가 복잡하게 느껴진다면, "문제 보고(Issue)" 기능을 통해 정보를 공유할 수 있습니다.

1. [Issues](https://github.com/OnlineLearningTutorials/mt7902_temp/issues) 탭으로 이동합니다.
2. **[New issue]** 를 클릭합니다.
3. 제목을 `[Document] Korean Installation Guide for Kernel 6.17+` 로 작성합니다.
4. 방금 작성한 `MT7902_installation_guide.md` 파일의 내용을 본문에 복사해서 붙여넣거나, 파일을 첨부합니다.
5. **[Submit new issue]** 를 클릭하면 관리자가 이를 확인하고 본문에 반영하거나 답변을 줄 것입니다.

---

## 💡 팁: 번역 제안 (English Version)
개발자가 외국인일 가능성이 높으므로, 가이드를 영문으로도 제공하면 채택될 확률이 훨씬 높습니다. 필요하시다면 작성한 한글 가이드를 영문으로 번역해드릴 수도 있습니다!
