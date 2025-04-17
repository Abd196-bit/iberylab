import React from 'react';
import styled from 'styled-components';

const AppContainer = styled.div`
  display: flex;
  height: 100vh;
  background-color: #1e1e1e;
  color: #d4d4d4;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', 'Consolas', 'source-code-pro', monospace;
`;

const Sidebar = styled.div`
  width: 250px;
  background-color: #252526;
  border-right: 1px solid #333;
  padding: 20px;
  font-family: inherit;
`;

const MainContent = styled.div`
  flex: 1;
  display: flex;
  flex-direction: column;
  font-family: inherit;
`;

const Editor = styled.div`
  flex: 1;
  padding: 20px;
  background-color: #1e1e1e;
  font-family: inherit;
`;

const App: React.FC = () => {
  return (
    <AppContainer>
      <Sidebar>
        <h2>Files</h2>
        {/* File explorer will go here */}
      </Sidebar>
      <MainContent>
        <Editor>
          <h2>Editor</h2>
          {/* Code editor will go here */}
        </Editor>
      </MainContent>
    </AppContainer>
  );
};

export default App; 